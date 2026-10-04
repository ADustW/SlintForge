#include "ForgeGrids.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <limits>
#include <iostream>

static std::vector<ForgeGridColumnData> forge_test_grid_columns() {
    return {{"name", "名称", 160, 0, 0, false, true, true, false, true},
        {"count", "数量", 80, 0, 0, true, true, true, false, false}};
}
static ForgeGridRowData forge_test_grid_row(const std::string& id, const std::string& parent, const std::string& text, double value) {
    return {id, parent, {{text, 0}, {std::to_string(static_cast<int>(value)), value}}, true, false, false};
}
static std::vector<ForgeGridRowData> forge_test_grid_rows() {
    auto rows = std::vector<ForgeGridRowData>{forge_test_grid_row("b", "root", "Beta", 10),
        forge_test_grid_row("a", "root", "alpha", 2), forge_test_grid_row("root", "", "Root", 12),
        forge_test_grid_row("z", "", "Archive", 24)};
    rows[3].disabled = true; return rows;
}
int main() {
    auto rows = forge_test_grid_rows(); auto columns = forge_test_grid_columns();
    auto scene = forge_prepare_grid(rows, columns, true, -1, false, "");
    assert(scene.rows.size() == 4 && scene.rows[0].row.id == "root" && scene.rows[1].depth == 1);
    scene = forge_prepare_grid(rows, columns, true, 1, false, "");
    assert(scene.rows[1].row.id == "a" && scene.rows[2].row.id == "b");
    rows[2].expanded = false;
    assert(forge_prepare_grid(rows, columns, true, -1, false, "").rows.size() == 2);
    scene = forge_prepare_grid(rows, columns, true, -1, false, " ALPHA ");
    assert(scene.rows.size() == 2 && scene.rows[1].row.id == "a");
    assert(forge_prepare_grid(rows, columns, true, -1, false, "absent").status == 1);
    auto invalid = rows; invalid[0].id = "root";
    assert(forge_prepare_grid(invalid, columns, true, -1, false, "").status == 2);
    invalid = rows; invalid[0].parent_id = "missing";
    assert(forge_prepare_grid(invalid, columns, true, -1, false, "").status == 2);
    invalid = rows; invalid[2].parent_id = "b";
    assert(forge_prepare_grid(invalid, columns, true, -1, false, "").status == 2);
    std::vector<ForgeGridRowData> deep;
    for (int i = 0; i < 10000; ++i) deep.push_back(forge_test_grid_row("n" + std::to_string(i), i == 0 ? "" : "n" + std::to_string(i - 1), "深层节点", i));
    scene = forge_prepare_grid(deep, columns, true, -1, false, "");
    assert(scene.rows.size() == 10000 && scene.rows.back().depth == 9999);
    rows = forge_test_grid_rows(); const std::vector<int> visible{2, 1, 0, 3};
    auto selected = forge_update_grid(rows, visible, 2, 0, 2);
    assert(selected[0].selected && selected[1].selected && selected[2].selected && !selected[3].selected);
    selected = forge_update_grid(selected, visible, 1, 1, -1);
    assert(!selected[1].selected && selected[0].selected);
    selected = forge_update_grid(selected, {1}, 12, -1, -1);
    assert(selected[1].selected && selected[0].selected);
    selected = forge_update_grid(selected, {1}, 4, -1, -1);
    assert(!selected[1].selected && selected[0].selected);
    selected = forge_update_grid(rows, visible, 0, 2, -1);
    assert(forge_update_grid(selected, visible, 11, 1, -1).size() == 1);
    assert(forge_update_grid(selected, visible, 11, 0, -1).size() == 3);
    assert(forge_step_grid({true, false, true, false}, -1, 1) == 1);
    assert(forge_step_grid({true, false, true, false}, 1, 1) == 3);
    assert(forge_step_grid({true, true}, -1, 1) == -1);
    assert(forge_step_grid({false, false}, 0, std::numeric_limits<int>::max()) == 1);
    auto edited = forge_commit_grid(rows, columns, 0, 1, " +3.5 ");
    assert(edited.status == 0 && edited.rows[0].cells[1].value == 3.5);
    for (const auto text : {"NaN", "inf", "1e100", "1e-100", "", "+-3", "2x"}) assert(forge_commit_grid(rows, columns, 0, 1, text).status == 2);
    assert(forge_commit_grid(rows, columns, 3, 1, "30").status == 1);
    columns[1].editable = false;
    assert(forge_commit_grid(rows, columns, 0, 1, "30").status == 1);
    columns[0].min_width = 100; columns[0].max_width = 200; columns[0].width = std::numeric_limits<double>::infinity();
    columns[1].width = 9000; columns[1].max_width = 240;
    scene = forge_prepare_grid({ForgeGridRowData{"empty"}}, columns, false, -1, false, "");
    assert(scene.rows[0].row.cells.size() == 2 && scene.total_width == 380 && scene.frozen_width == 140);
    columns[0].hidden = true;
    assert(forge_prepare_grid(rows, columns, false, -1, false, "").columns[0].source_index == 1);
    std::filesystem::create_directories("build/grids-qa");
    std::ofstream output("build/grids-qa/cpp-contract.csv"); columns = forge_test_grid_columns();
    for (int index = 0; index < 10; ++index) {
        auto data = rows;
        if (index == 4) data[2].expanded = false;
        if (index == 5) data[0].selected = true;
        if (index == 6) data[0].parent_id = "invalid";
        if (index == 7) columns[1].hidden = true;
        if (index == 8) data[0].cells.clear();
        if (index == 9) data[1].cells[1].value = std::numeric_limits<double>::quiet_NaN();
        scene = forge_prepare_grid(data, columns, index != 0, index == 1 || index == 2 ? 1 : -1, index == 2, index == 3 ? "beta" : "");
        output << "scene," << index << ',' << scene.status << ',' << scene.total_width << ',' << scene.frozen_width << ',' << scene.selected_count << ',' << scene.all_visible_selected << '\n';
        for (const auto& c : scene.columns) output << "column," << index << ',' << c.column.id << ',' << c.source_index << ',' << c.x << ',' << c.width << ',' << c.column.frozen << '\n';
        for (const auto& r : scene.rows) output << "row," << index << ',' << r.row.id << ',' << r.source_index << ',' << r.depth << ',' << r.has_children << ',' << r.expanded << ',' << r.row.disabled << '\n';
    }
    assert(output.good()); std::cout << "GRID_MATH_OK cases=10 deep=10000 selection=passed editing=passed\n";
}
