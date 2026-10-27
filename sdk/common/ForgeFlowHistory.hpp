#pragma once
#include <list>
#include <memory>

// 历史只持有 CBOR 编码的双向差量，不持有文档快照或渲染缓存。
// 字节计费包括有效载荷、条目对象和链表链接；不代表进程或分配器的总内存。
class ForgeFlowHistory {
    struct Entry {
        std::unique_ptr<std::uint8_t[]> data;
        std::size_t size = 0;
        std::size_t bytes() const { return size + sizeof(Entry) + 2 * sizeof(void*); }
    };
    std::list<Entry> undo_, redo_;
    std::size_t undo_bytes_ = 0, redo_bytes_ = 0;
    std::size_t max_steps_ = 100, max_bytes_ = 32 * 1024 * 1024;
    std::uint64_t evicted_ = 0, barriers_ = 0;

    static bool metadata_key(const std::string& key) {
        return key != "nodes" && key != "edges" && key != "viewport";
    }
    static ForgeFlowJson metadata(const ForgeFlowJson& document) {
        auto result = ForgeFlowJson::object();
        for (const auto& [key, value] : document.items()) if (metadata_key(key)) result[key] = value;
        return result;
    }
    static ForgeFlowJson entity_delta(const ForgeFlowJson& before, const ForgeFlowJson& after) {
        std::unordered_map<std::string, std::size_t> old_ids, new_ids;
        for (std::size_t i = 0; i < before.size(); ++i) old_ids.emplace(before[i].at("id").get<std::string>(), i);
        for (std::size_t i = 0; i < after.size(); ++i) new_ids.emplace(after[i].at("id").get<std::string>(), i);
        auto result = ForgeFlowJson::object();
        auto old_common = ForgeFlowJson::array(), new_common = ForgeFlowJson::array();
        for (std::size_t i = 0; i < before.size(); ++i) {
            const auto id = before[i].at("id").get<std::string>();
            const auto found = new_ids.find(id);
            if (found == new_ids.end()) result["removed"].push_back({i, before[i]});
            else {
                old_common.push_back(id);
                const auto& next = after[found->second];
                if (before[i] != next) result["changed"].push_back({id, ForgeFlowJson::diff(before[i], next), ForgeFlowJson::diff(next, before[i])});
            }
        }
        for (std::size_t i = 0; i < after.size(); ++i) {
            const auto id = after[i].at("id").get<std::string>();
            if (!old_ids.contains(id)) result["added"].push_back({i, after[i]});
            else new_common.push_back(id);
        }
        // 普通插入、删除只记录受影响元素及位置；真正重排才保存 ID 顺序。
        if (old_common != new_common) {
            auto old_order = ForgeFlowJson::array(), new_order = ForgeFlowJson::array();
            for (const auto& item : before) old_order.push_back(item.at("id"));
            for (const auto& item : after) new_order.push_back(item.at("id"));
            result["order"] = {old_order, new_order};
        }
        return result;
    }
    static void apply_entities(ForgeFlowJson& entities, const ForgeFlowJson& delta, bool forward) {
        const auto* removed = forward ? "removed" : "added";
        const auto* added = forward ? "added" : "removed";
        if (delta.contains(removed)) {
            std::set<std::string> ids;
            for (const auto& item : delta[removed]) ids.insert(item[1].at("id").get<std::string>());
            auto& array = entities.get_ref<ForgeFlowJson::array_t&>();
            std::erase_if(array, [&](const auto& item) { return ids.contains(item.at("id").template get<std::string>()); });
        }
        std::unordered_map<std::string, std::size_t> indices;
        for (std::size_t i = 0; i < entities.size(); ++i) indices.emplace(entities[i].at("id").get<std::string>(), i);
        if (delta.contains("changed")) for (const auto& change : delta["changed"]) {
            auto& entity = entities.at(indices.at(change[0].get<std::string>()));
            entity = entity.patch(change[forward ? 1 : 2]);
        }
        if (delta.contains(added)) for (const auto& item : delta[added]) {
            auto& array = entities.get_ref<ForgeFlowJson::array_t&>();
            const auto position = item[0].get<std::size_t>();
            if (position > array.size()) throw std::runtime_error("历史元素位置无效");
            array.insert(array.begin() + position, item[1]);
        }
        if (delta.contains("order")) {
            indices.clear();
            for (std::size_t i = 0; i < entities.size(); ++i) indices.emplace(entities[i].at("id").get<std::string>(), i);
            auto ordered = ForgeFlowJson::array();
            for (const auto& id : delta["order"][forward ? 1 : 0]) ordered.push_back(std::move(entities.at(indices.at(id.get<std::string>()))));
            entities = std::move(ordered);
        }
    }
    void trim() {
        // 撤销移除最早一步，重做移除最远一步，始终保留从当前文档可达的连续链。
        while (undo_.size() + redo_.size() > max_steps_ || undo_bytes_ + redo_bytes_ > max_bytes_) {
            if (!undo_.empty()) { undo_bytes_ -= undo_.front().bytes(); undo_.pop_front(); }
            else { redo_bytes_ -= redo_.front().bytes(); redo_.pop_front(); }
            ++evicted_;
        }
    }
public:
    bool can_undo() const { return !undo_.empty(); }
    bool can_redo() const { return !redo_.empty(); }
    void reset() {
        undo_.clear(); redo_.clear(); undo_bytes_ = redo_bytes_ = 0; evicted_ = barriers_ = 0;
    }
    ForgeFlowJson status() const {
        return {{"undo_count", undo_.size()}, {"redo_count", redo_.size()}, {"history_limit", max_steps_},
            {"history_bytes", undo_bytes_ + redo_bytes_}, {"undo_bytes", undo_bytes_}, {"redo_bytes", redo_bytes_},
            {"history_budget_bytes", max_bytes_}, {"history_evicted", evicted_}, {"history_barriers", barriers_},
            {"history_accounting", "cbor_payload_and_entry"}};
    }
    void configure(const ForgeFlowJson& options) {
        if (!options.is_object()) throw std::runtime_error("历史预算必须为对象");
        auto steps = max_steps_, bytes = max_bytes_;
        for (const auto& [key, value] : options.items()) {
            if (key != "max_steps" && key != "max_bytes") throw std::runtime_error("未知历史预算选项");
            const auto limit = key == "max_steps" ? 10000ull : 1024ull * 1024 * 1024;
            if (!value.is_number_integer() || value < 0 || value > limit) throw std::runtime_error("历史预算必须为范围内的非负整数");
            (key == "max_steps" ? steps : bytes) = value.get<std::size_t>();
        }
        max_steps_ = steps; max_bytes_ = bytes; trim();
    }
    void record(const ForgeFlowJson& before, const ForgeFlowJson& after) {
        auto delta = ForgeFlowJson::object();
        for (const auto* key : {"nodes", "edges"}) if (before.at(key) != after.at(key)) {
            auto changes = entity_delta(before.at(key), after.at(key));
            if (!changes.empty()) delta[key] = std::move(changes);
        }
        const auto old_metadata = metadata(before), new_metadata = metadata(after);
        if (old_metadata != new_metadata) delta["metadata"] = {ForgeFlowJson::diff(old_metadata, new_metadata), ForgeFlowJson::diff(new_metadata, old_metadata)};
        if (delta.empty()) return;
        // 超预算操作正常生效，但清空两端历史形成边界，禁止跨过未记录的编辑。
        const auto encoded = ForgeFlowJson::to_cbor(delta);
        Entry entry; entry.size = encoded.size();
        if (max_steps_ == 0 || entry.bytes() > max_bytes_) {
            evicted_ += undo_.size() + redo_.size(); ++barriers_;
            undo_.clear(); redo_.clear(); undo_bytes_ = redo_bytes_ = 0; return;
        }
        entry.data = std::make_unique<std::uint8_t[]>(entry.size);
        std::copy(encoded.begin(), encoded.end(), entry.data.get());
        const auto bytes = entry.bytes();
        undo_.push_back(std::move(entry)); undo_bytes_ += bytes;
        redo_.clear(); redo_bytes_ = 0; trim();
    }
    bool restore(ForgeFlowJson& document, bool forward) {
        auto& from = forward ? redo_ : undo_;
        auto& to = forward ? undo_ : redo_;
        if (from.empty()) return false;
        const auto& entry = from.back();
        const auto delta = ForgeFlowJson::from_cbor(entry.data.get(), entry.data.get() + entry.size);
        // 先完成差量应用再交换，解码或应用失败不消耗历史，也不留下部分文档。
        auto restored = document;
        for (const auto* key : {"nodes", "edges"}) if (delta.contains(key)) apply_entities(restored[key], delta[key], forward);
        if (delta.contains("metadata")) {
            const auto next = metadata(restored).patch(delta["metadata"][forward ? 0 : 1]);
            for (auto it = restored.begin(); it != restored.end();) if (metadata_key(it.key())) it = restored.erase(it); else ++it;
            for (const auto& [key, value] : next.items()) restored[key] = value;
        }
        document.swap(restored);
        const auto bytes = entry.bytes();
        (forward ? redo_bytes_ : undo_bytes_) -= bytes;
        (forward ? undo_bytes_ : redo_bytes_) += bytes;
        to.splice(to.end(), from, std::prev(from.end()));
        return true;
    }
};
