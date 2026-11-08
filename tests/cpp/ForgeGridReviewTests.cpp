// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

#include "ForgeGrids.hpp"
#include <chrono>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <limits>

static void forge_grid_review_require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
    const auto directory=std::filesystem::path("build/plot-qa/grid-review");std::filesystem::create_directories(directory);std::ofstream csv(directory/(argc>1?argv[1]:"cpp-performance.csv"));csv<<"host,rows,shape,operation,ms,visible\n";
    const std::vector<ForgeGridColumnData> columns{{"text","名称",160,40,300,false,true,true,false,false},{"value","数值",100,40,200,true,true,true,false,false}};
    for(int count:{10000,100000})for(bool tree:{false,true}){std::vector<ForgeGridRowData> rows;rows.reserve(count);for(int i=0;i<count;++i){const std::string key=std::to_string((count-i)*17);const std::string text=(i%2?"A":"a")+key+std::string(70,'X');rows.push_back({std::to_string(i),tree&&i?std::to_string((i-1)/4):"",{{text,0},{key,static_cast<double>(count-i)}},true,false,i%23==0});}
        for(int operation=0;operation<4;++operation){const auto start=std::chrono::steady_clock::now();const auto scene=forge_prepare_grid(rows,columns,tree,operation==0?-1:operation==1?0:1,operation==2,operation==3?"A17":"");const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();forge_grid_review_require(scene.status!=2,"有效树模型被拒绝");if(operation<3)forge_grid_review_require(scene.rows.size()==rows.size(),"准备视图丢失行");csv<<"cpp,"<<count<<','<<(tree?"tree":"flat")<<','<<operation<<','<<ms<<','<<scene.rows.size()<<'\n';std::cout<<"GRID_REVIEW_PERF host=cpp rows="<<count<<" tree="<<tree<<" op="<<operation<<" ms="<<ms<<'\n';}
    }
    const std::vector<ForgeGridRowData> equal{{"a","",{{"AbC",0},{"nan",std::numeric_limits<double>::quiet_NaN()}},true,false,false},{"b","",{{"abc",0},{"one",1}},true,false,false},{"c","",{{"ABC",0},{"missing",0}},true,false,false}};
    const auto stable=forge_prepare_grid(equal,columns,false,0,true," \t");forge_grid_review_require(stable.rows[0].row.id=="a"&&stable.rows[1].row.id=="b"&&stable.rows[2].row.id=="c","等值文本排序不稳定");const auto finite=forge_prepare_grid(equal,columns,false,1,true,"");forge_grid_review_require(finite.rows.back().row.id=="a","降序的无效数值未置后");std::cout<<"GRID_REVIEW_TESTS_OK host=cpp\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
