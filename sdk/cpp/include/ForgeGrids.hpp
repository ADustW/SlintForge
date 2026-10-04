#pragma once
#include <slint.h>
#include <algorithm>
#include <string>
#include <vector>

// 第一方类型使用全局唯一前缀，与共享 Slint 表格契约一一对应。
struct ForgeGridCellData { std::string text; double value = 0; };
struct ForgeGridColumnData {
    std::string id, title;
    double width = 0, min_width = 0, max_width = 0;
    bool numeric = false, sortable = false, editable = false, hidden = false, frozen = false;
};
struct ForgeGridRowData {
    std::string id, parent_id;
    std::vector<ForgeGridCellData> cells;
    bool expanded = false, selected = false, disabled = false;
};
struct ForgeGridViewColumnData { ForgeGridColumnData column; int source_index = 0; double x = 0, width = 0; };
struct ForgeGridViewRowData { ForgeGridRowData row; int source_index = 0, depth = 0; bool has_children = false, expanded = false; };
struct ForgeGridSceneData {
    int status = 0, selected_count = 0;
    std::vector<ForgeGridViewRowData> rows;
    std::vector<ForgeGridViewColumnData> columns;
    double total_width = 0, frozen_width = 0;
    bool all_visible_selected = false;
};
struct ForgeGridEditData { int status = 0; std::vector<ForgeGridRowData> rows; };

ForgeGridSceneData forge_prepare_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<ForgeGridColumnData>& columns, bool tree, int sort_column, bool descending, const std::string& filter);
std::vector<ForgeGridRowData> forge_update_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<int>& visible, int operation, int target, int anchor);
ForgeGridEditData forge_commit_grid(const std::vector<ForgeGridRowData>& rows,
    const std::vector<ForgeGridColumnData>& columns, int row, int column, const std::string& text);
int forge_step_grid(const std::vector<bool>& disabled, int current, int delta);

template<typename Row> std::vector<ForgeGridRowData> forge_grid_read_rows(const std::shared_ptr<slint::Model<Row>>& model) {
    std::vector<ForgeGridRowData> values;
    if (!model) return values;
    for (std::size_t i = 0; i < model->row_count(); ++i) {
        const auto row = model->row_data(i);
        if (!row) continue;
        ForgeGridRowData value{};
        value.id = std::string(row->id); value.parent_id = std::string(row->parent_id);
        value.expanded = row->expanded; value.selected = row->selected; value.disabled = row->disabled;
        if (row->cells) for (std::size_t c = 0; c < row->cells->row_count(); ++c) {
            const auto cell = row->cells->row_data(c);
            if (cell) value.cells.push_back({std::string(cell->text), cell->value});
        }
        values.push_back(std::move(value));
    }
    return values;
}
template<typename Column> std::vector<ForgeGridColumnData> forge_grid_read_columns(const std::shared_ptr<slint::Model<Column>>& model) {
    std::vector<ForgeGridColumnData> values;
    if (!model) return values;
    for (std::size_t i = 0; i < model->row_count(); ++i) {
        const auto c = model->row_data(i);
        if (c) values.push_back({std::string(c->id), std::string(c->title), c->width, c->min_width, c->max_width,
            c->numeric, c->sortable, c->editable, c->hidden, c->frozen});
    }
    return values;
}
template<typename Cell> std::shared_ptr<slint::Model<Cell>> forge_grid_write_cells(const std::vector<ForgeGridCellData>& cells) {
    std::vector<Cell> output;
    for (const auto& value : cells) { Cell c{}; c.text = value.text; c.value = static_cast<float>(value.value); output.push_back(c); }
    return std::make_shared<slint::VectorModel<Cell>>(std::move(output));
}
template<typename Row, typename Cell> std::shared_ptr<slint::Model<Row>> forge_grid_write_rows(const std::vector<ForgeGridRowData>& rows) {
    std::vector<Row> output;
    for (const auto& value : rows) {
        Row r{}; r.id = value.id; r.parent_id = value.parent_id; r.cells = forge_grid_write_cells<Cell>(value.cells);
        r.expanded = value.expanded; r.selected = value.selected; r.disabled = value.disabled; output.push_back(r);
    }
    return std::make_shared<slint::VectorModel<Row>>(std::move(output));
}
template<typename Column> Column forge_grid_write_column(const ForgeGridColumnData& value) {
    Column c{}; c.id = value.id; c.title = value.title; c.width = static_cast<float>(value.width);
    c.min_width = static_cast<float>(value.min_width); c.max_width = static_cast<float>(value.max_width);
    c.numeric = value.numeric; c.sortable = value.sortable; c.editable = value.editable;
    c.hidden = value.hidden; c.frozen = value.frozen; return c;
}

template<typename Services, typename Scene, typename ViewRow, typename ViewColumn, typename Row, typename Cell,
    typename Column, typename Edit, typename Component>
void forge_bind_grid_services(const slint::ComponentHandle<Component>& ui) {
    const auto& service = ui->template global<Services>();
    service.on_prepare([](const auto& rows, const auto& columns, bool tree, int sort, bool descending,
        const slint::SharedString& filter, int) {
        const auto data = forge_prepare_grid(forge_grid_read_rows<Row>(rows), forge_grid_read_columns<Column>(columns),
            tree, sort, descending, std::string(filter));
        Scene scene{}; scene.status = data.status; scene.total_width = static_cast<float>(data.total_width);
        scene.frozen_width = static_cast<float>(data.frozen_width); scene.selected_count = data.selected_count;
        scene.all_visible_selected = data.all_visible_selected;
        std::vector<ViewRow> view_rows;
        for (const auto& v : data.rows) {
            ViewRow r{}; r.id = v.row.id; r.parent_id = v.row.parent_id; r.cells = forge_grid_write_cells<Cell>(v.row.cells);
            r.source_index = v.source_index; r.depth = v.depth; r.has_children = v.has_children;
            r.expanded = v.expanded; r.selected = v.row.selected; r.disabled = v.row.disabled; view_rows.push_back(r);
        }
        scene.rows = std::make_shared<slint::VectorModel<ViewRow>>(std::move(view_rows));
        std::vector<ViewColumn> view_columns;
        for (const auto& v : data.columns) {
            ViewColumn c{}; c.column = forge_grid_write_column<Column>(v.column); c.source_index = v.source_index;
            c.x = static_cast<float>(v.x); c.width = static_cast<float>(v.width); view_columns.push_back(c);
        }
        scene.columns = std::make_shared<slint::VectorModel<ViewColumn>>(std::move(view_columns));
        return scene;
    });
    service.on_update([](const auto& rows, const auto& view, int operation, int target, int anchor) {
        std::vector<int> visible;
        if (view) for (std::size_t i = 0; i < view->row_count(); ++i) { const auto r = view->row_data(i); if (r) visible.push_back(r->source_index); }
        const auto source = forge_grid_read_rows<Row>(rows);
        const auto updated = forge_update_grid(source, visible, operation, target, anchor);
        // 无变化的选择操作保留模型，防止第二次点击销毁原生鼠标抓取目标。
        if (source.size() == updated.size() && std::equal(source.begin(), source.end(), updated.begin(),
            [](const auto& a, const auto& b) { return a.selected == b.selected && a.expanded == b.expanded; })) return rows;
        return forge_grid_write_rows<Row, Cell>(updated);
    });
    service.on_commit([](const auto& rows, const auto& columns, int row, int column, const slint::SharedString& text) {
        const auto data = forge_commit_grid(forge_grid_read_rows<Row>(rows), forge_grid_read_columns<Column>(columns), row, column, std::string(text));
        Edit result{}; result.status = data.status; result.rows = forge_grid_write_rows<Row, Cell>(data.rows); return result;
    });
    service.on_find_row([](const auto& rows, const slint::SharedString& id) {
        if (rows) for (std::size_t i = 0; i < rows->row_count(); ++i) { const auto r = rows->row_data(i); if (r && r->id == id) return static_cast<int>(i); }
        return -1;
    });
    service.on_step([](const auto& rows, int current, int delta) {
        std::vector<bool> disabled;
        if (rows) for (std::size_t i = 0; i < rows->row_count(); ++i) { const auto r = rows->row_data(i); disabled.push_back(!r || r->disabled); }
        return forge_step_grid(disabled, current, delta);
    });
    service.on_find_column([](const auto& columns, int source) {
        if (columns) for (std::size_t i = 0; i < columns->row_count(); ++i) { const auto c = columns->row_data(i); if (c && c->source_index == source) return static_cast<int>(i); }
        return -1;
    });
    service.on_filter_active([](const slint::SharedString& filter){return std::string_view(filter).find_first_not_of(" \t\n\r\f\v")!=std::string_view::npos;});
    service.set_connected(true);
}
