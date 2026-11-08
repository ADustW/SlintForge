// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGalleryGrids.hpp"
#include "ForgeGrids.hpp"
#include <algorithm>
#include <locale>
#include <sstream>
#include <stdexcept>

void forge_gallery_connect_grids(const slint::ComponentHandle<GalleryWindow>& ui) {
    forge_bind_grid_services<ForgeGridServices, ForgeGridScene, ForgeGridViewRow, ForgeGridViewColumn,
        ForgeGridRow, ForgeGridCell, ForgeGridColumn, ForgeGridEditResult>(ui);
    const auto weak = slint::ComponentWeakHandle<GalleryWindow>(ui);
    ui->global<GalleryGridDemoModels>().on_action([weak](bool tree, const slint::SharedString& action,
        const std::shared_ptr<slint::Model<ForgeGridRow>>& rows) -> std::shared_ptr<slint::Model<ForgeGridRow>> {
        const auto ui = weak.lock();
        if (!ui) return std::make_shared<slint::VectorModel<ForgeGridRow>>();
        const auto& models = (*ui)->global<GalleryGridDemoModels>();
        const auto source = tree ? models.get_tree_seed() : models.get_flat_seed();
        const auto seed = source->row_data(0).value();
        const auto make_row = [&](std::size_t index, bool large) {
            auto row = seed;
            const auto group = index / 1000;
            const bool is_root = index % 1000 == 0;
            const auto group_id = "group-" + std::to_string(group);
            row.id = tree && large ? (is_root ? group_id : group_id + "/" + std::to_string(index)) : "new-" + std::to_string(index);
            row.parent_id = tree && large && !is_root ? group_id : "";
            row.expanded = true; row.selected = false; row.disabled = false;
            std::vector<ForgeGridCell> cells;
            if (row.cells) for (std::size_t i = 0; i < row.cells->row_count(); ++i) cells.push_back(row.cells->row_data(i).value());
            cells[0].text = std::string(tree && large && is_root ? models.get_group_label() : models.get_new_label()) + " " + std::to_string(index + 1);
            cells[1].value = static_cast<float>(index % 200); cells[1].text = std::to_string(index % 200);
            row.cells = std::make_shared<slint::VectorModel<ForgeGridCell>>(std::move(cells)); return row;
        };
        std::vector<ForgeGridRow> result;
        if (rows) for (std::size_t i = 0; i < rows->row_count(); ++i) result.push_back(rows->row_data(i).value());
        if (action == "large") {
            result.clear(); result.reserve(10000);
            for (std::size_t i = 0; i < 10000; ++i) result.push_back(make_row(i, true));
        } else if (action == "append") {
            auto index = result.size();
            while (std::any_of(result.begin(), result.end(), [&](const auto& r) { return std::string(r.id) == "new-" + std::to_string(index); })) ++index;
            result.push_back(make_row(index, false));
        } else if (action == "update" && !result.empty()) {
            std::vector<ForgeGridCell> cells;
            if (result[0].cells) for (std::size_t i = 0; i < result[0].cells->row_count(); ++i) cells.push_back(result[0].cells->row_data(i).value());
            if (cells.size() > 1) {
                cells[1].value += 10;
                std::ostringstream text; text.imbue(std::locale::classic()); text.precision(7);
                text << cells[1].value; cells[1].text = text.str();
            }
            result[0].cells = std::make_shared<slint::VectorModel<ForgeGridCell>>(std::move(cells));
        }
        return std::make_shared<slint::VectorModel<ForgeGridRow>>(std::move(result));
    });
}

void forge_gallery_verify_grids(const slint::ComponentHandle<GalleryWindow>& ui) {
    const auto& models = ui->global<GalleryGridDemoModels>();
    ForgeGridColumn name{}; name.id = "name"; name.editable = true;
    ForgeGridColumn count{}; count.id = "count"; count.numeric = true; count.editable = true;
    const auto columns = std::make_shared<slint::VectorModel<ForgeGridColumn>>(std::vector<ForgeGridColumn>{name, count});
    for (const bool tree : {false, true}) {
        const auto seed = tree ? models.get_tree_seed() : models.get_flat_seed();
        const auto appended = models.invoke_action(tree, "append", seed);
        if (appended->row_count() != seed->row_count() + 1) throw std::runtime_error("grid append");
        const auto large = models.invoke_action(tree, "large", seed);
        const auto scene = forge_prepare_grid(forge_grid_read_rows<ForgeGridRow>(large),
            forge_grid_read_columns<ForgeGridColumn>(columns), tree, -1, false, "");
        if (scene.status != 0 || scene.rows.size() != 10000) throw std::runtime_error("grid large model");
    }
    const auto edited = ui->global<ForgeGridServices>().invoke_commit(models.get_flat_seed(), columns, 0, 1, "3.5");
    if (edited.status != 0) throw std::runtime_error("grid seed edit");
    const auto updated = models.invoke_action(false, "update", edited.rows);
    const auto cell = updated->row_data(0)->cells->row_data(1).value();
    if (cell.value != 13.5f || cell.text != "13.5") throw std::runtime_error("grid decimal update");
}
