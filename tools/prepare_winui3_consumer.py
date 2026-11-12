# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

"""准备隔离的 WinUI 3/ForgePlot SDK 消费者；只写入 build 验证目录，不组装发布包。"""
from pathlib import Path
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools/windows"))
from release_payload import interface_files, interface_text

stage = ROOT / "build/winui3/sdk"
consumer = ROOT / "build/winui3/consumer"
consumer.mkdir(parents=True, exist_ok=True)
for source, relative in interface_files():
    destination = stage / Path(*relative.parts[1:])
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(interface_text(source), encoding="utf-8")
for source, destination in (
    (ROOT / "target/debug/slintforge_runtime.dll", stage / "rust/runtime/slintforge_runtime.dll"),
    (ROOT / "build/ui-compiler-release/release/slintforge-ui-compiler.exe", stage / "rust/bin/slintforge-ui-compiler.exe"),
):
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)

files = {
    "consumer.slint": '''import {Button, LineEdit} from "std-widgets.slint";
import {ForgeTheme, ForgeMetrics, ForgeTypography, ForgePlot, ForgePlotScene, ForgePlotExportResult} from "@slintforge";
export {ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult} from "@slintforge";
export component ConsumerWindow inherits Window {
    width:720px;height:540px;default-font-family:ForgeTypography.resolved-family;default-font-size:ForgeTypography.body;
    in-out property<bool> dark:false;
    changed dark=>{ForgeTheme.color-scheme=root.dark?ColorScheme.dark:ColorScheme.light;}
    out property<bool> theme-ok:ForgeTheme.accent-background==(root.dark?#60cdff:#005fb8)&&ForgeMetrics.control-height==32px;
    out property<ForgePlotScene> scene:plot.scene;
    callback export-chart(string,string)->ForgePlotExportResult;
    export-chart(path,format)=>{return plot.export-file(path,format);}
    VerticalLayout {
        HorizontalLayout {Button {text:"标准按钮";}LineEdit {text:"草稿";}}
        plot:=ForgePlot {series:[{id:7,kind:pie,inner-radius:0.5,points:[{y:2},{y:3},{y:5}],point-styles:[{index:1,color:#00b7c3}]}];panels:[{id:0,width:1,height:1,categories:["研发","设计","测试"]}];}
    }
}
''',
    "main.cpp": '''#include "consumer.h"
#include "ForgePlot.hpp"
#include <iostream>
#include <stdexcept>
int main(){
    auto ui=ConsumerWindow::create();
    forge_bind_plot_services<ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult>(ui);
    slint::Timer timer;int phase=0,token=0;
    timer.start(slint::TimerMode::Repeated,std::chrono::milliseconds(250),[&]{
        const auto scene=ui->get_scene();
        if(scene.status!=0||scene.rendered_points!=3||!ui->get_theme_ok())throw std::runtime_error("安装 SDK 的主题或图表契约错误");
        if(phase++==0){token=scene.token;ui->set_dark(true);}else{
            if(scene.token!=token)throw std::runtime_error("切换主题重建了数据实例");
            if(!ui->invoke_export_chart("consumer-cpp.svg","svg").ok||!ui->invoke_export_chart("consumer-cpp.png","png").ok)throw std::runtime_error("导出失败");
            std::cout<<"INSTALLED_WINUI3_CONSUMER_OK host=cpp abi=2\\n";slint::quit_event_loop();
        }
    });ui->run();
}
''',
    "main.rs": '''include!(env!("SLINTFORGE_GENERATED_UI"));
use slint::ComponentHandle;
fn main(){
    let ui=ConsumerWindow::new().unwrap();
    slintforge::bind_plot_services!(ui,ForgePlotServices,ForgePlotScene,ForgePlotAxis,ForgePlotRange,ForgePlotRect,ForgePlotHit,ForgePlotSelection,ForgePlotExportResult);
    let timer=slint::Timer::default();let weak=ui.as_weak();let phase=std::cell::Cell::new(0);let token=std::cell::Cell::new(0);
    timer.start(slint::TimerMode::Repeated,std::time::Duration::from_millis(250),move||{
        let ui=weak.unwrap();let scene=ui.get_scene();assert_eq!(scene.status,0,"{}",scene.error);assert_eq!(scene.rendered_points,3);assert!(ui.get_theme_ok());
        if phase.replace(1)==0 {token.set(scene.token);ui.set_dark(true);}else{
            assert_eq!(scene.token,token.get());assert!(ui.invoke_export_chart("consumer-rust.svg".into(),"svg".into()).ok);assert!(ui.invoke_export_chart("consumer-rust.png".into(),"png".into()).ok);
            println!("INSTALLED_WINUI3_CONSUMER_OK host=rust abi=2");slint::quit_event_loop().unwrap();
        }
    });ui.run().unwrap();
}
''',
    "CMakeLists.txt": '''cmake_minimum_required(VERSION 3.21)
project(ForgeWinUI3Consumer LANGUAGES CXX)
add_compile_options(/utf-8)
find_package(SlintForge CONFIG REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE SlintForge::Sdk)
slintforge_target_sources(consumer consumer.slint)
''',
    "Cargo.toml": '''[package]
name="slintforge-winui3-consumer"
version="0.0.0"
edition="2021"
[workspace]
[[bin]]
name="slintforge-winui3-consumer"
path="main.rs"
[dependencies]
slint="=1.18.0"
slintforge={path="../sdk/rust"}
[build-dependencies]
slintforge-build={path="../sdk/rust-build"}
''',
    "build.rs": 'fn main(){slintforge_build::compile_ui("consumer.slint").unwrap();}\n',
}
for name, content in files.items():
    (consumer / name).write_text(content, encoding="utf-8")
print(consumer)
