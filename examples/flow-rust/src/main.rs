slint::include_modules!();
mod performance;
mod qa;
mod sdk_contract;
mod card_export;
fn main() -> Result<(), Box<dyn std::error::Error>> {
    if std::env::args().any(|arg| arg == "--sdk-contract") {
        return sdk_contract::run();
    }
    #[cfg(target_os = "windows")]
    if std::env::var_os("WGPU_BACKEND").is_none() {
        std::env::set_var("WGPU_BACKEND", "dx12");
    }
    let backend = std::env::var("SLINT_BACKEND").unwrap_or_else(|_| "winit-skia".into());
    slint::BackendSelector::new()
        .backend_name("winit".into())
        .renderer_name(backend.trim_start_matches("winit-").into())
        .select()?;
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
