// Copyright © 2026 陕西禾又科技有限责任公司
// SPDX-License-Identifier: GPL-3.0-only

slint::include_modules!();
mod performance;
mod qa;
mod sdk_contract;
mod card_export;
fn main() -> Result<(), Box<dyn std::error::Error>> {
    if std::env::args().any(|arg| arg == "--sdk-contract") {
        return sdk_contract::run();
    }
    slintforge::configure_rendering(Default::default())?;
    let ui = ForgeFlowWindow::new()?;
    slintforge::bind_flow_services!(
        ui,
        ForgeFlowServices,
        ForgeFlowPointData,
        ForgeFlowPortData,
        ForgeFlowNodeData,
        ForgeFlowEdgeData,
        ForgeFlowViewport,
        ForgeFlowRect,
        ForgeFlowAction,
        ForgeFlowState
    );
    ui.global::<ForgeFlowServices>().on_export_card(|node|card_export::content(node.as_str()).into());
    let verification = std::env::args()
        .any(|arg| arg == "--smoke")
        .then(|| qa::start(&ui));
    let _benchmark = if std::env::args().any(|arg| arg == "--benchmark") {
        Some(performance::start(
            &ui,
            std::env::args().any(|arg| arg == "--complex"),
        )?)
    } else {
        None
    };
    ui.run()?;
    if verification
        .as_ref()
        .is_some_and(|(_, failed)| failed.get())
    {
        return Err("ForgeFlow 窗口验收失败".into());
    }
    Ok(())
}
