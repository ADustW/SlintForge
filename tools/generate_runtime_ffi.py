"""从公开数据模型生成 C 视图与双端转换，固定 DLL ABI 的字段顺序。"""
import argparse
import hashlib
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULES = ('lib', 'icons', 'staggered_layout', 'charts', 'grids', 'widgets', 'capture')
OPS = [
    ('RouterNew', [('ids','Vec<String>'),('initial','String')]),
    ('RouterClone', [('handle','u64')]),
    ('RouterGo', [('handle','u64'),('id','String'),('params','Vec<RouteParam>')]),
    ('RouterReplace', [('handle','u64'),('id','String'),('params','Vec<RouteParam>')]),
    ('RouterBack', [('handle','u64')]), ('RouterFree', [('handle','u64')]),
    ('NavNew', [('items','Vec<NavItem>')]), ('NavClone', [('handle','u64')]),
    ('NavToggle', [('handle','u64'),('id','String')]),
    ('NavExpand', [('handle','u64'),('id','String'),('expanded','bool')]),
    ('NavVisible', [('handle','u64')]), ('NavFree', [('handle','u64')]),
    ('IconsAll', []), ('IconsFind', [('name','String')]),
    ('IconsSearch', [('query','String'),('offset','usize'),('limit','usize')]),
    ('Arrange', [('items','Vec<StaggeredInput>'),('columns','usize'),('gap','f32'),('padding','f32')]),
    ('ColorParse', [('text','String')]), ('ColorText', [('color','Color')]),
    ('ScreenColor', []), ('ScreenInput', []),
    ('ChartColor', [('index','usize')]),
    ('ChartPrepare', [('kind','i32'),('series','Vec<ChartSeriesData>'),('slices','Vec<ChartSliceData>'),('categories','Vec<String>'),('options','ChartOptionsData')]),
    ('ChartHit', [('marks','Vec<ChartMarkData>'),('x','f64'),('y','f64'),('width','f64'),('height','f64'),('inner_radius','f64')]),
    ('GridPrepare', [('rows','Vec<GridRowData>'),('columns','Vec<GridColumnData>'),('tree','bool'),('sort_column','i32'),('descending','bool'),('filter','String')]),
    ('GridUpdate', [('rows','Vec<GridRowData>'),('visible','Vec<i32>'),('operation','i32'),('target','i32'),('anchor','i32')]),
    ('GridCommit', [('rows','Vec<GridRowData>'),('columns','Vec<GridColumnData>'),('row','i32'),('column','i32'),('text','String')]),
    ('GridStep', [('disabled','Vec<bool>'),('current','i32'),('delta','i32')]),
    ('Qr', [('text','String')]), ('Highlight', [('code','String'),('language','String')]),
    ('Backdrop', [('pixels','Pixels'),('radius','i32'),('glass','bool')]),
    ('Crop', [('pixels','Pixels'),('rect','CaptureRect'),('scale','f32')]),
    ('SavePng', [('pixels','Pixels'),('path','Path')]),
    ('DecodeImage', [('bytes','Vec<u8>')]),
    ('SaveImage', [('pixels','Pixels'),('path','Path'),('format','i32')]),
    ('MergeSelection', [('current','Vec<(i32,i32,i32)>'),('incoming','Vec<(i32,i32,i32)>'),('mode','i32')]),
    ('FlowNew', [('mode','String')]),
    ('FlowCall', [('handle','u64'),('command','String')]),
    ('FlowSnapshot', [('handle','u64')]),
    ('FlowExport', [('handle','u64')]),
    ('FlowFree', [('handle','u64')]),
]
EXTRA = {'RouterSnapshot': [('handle','u64'),('current','RouteState'),('history_len','usize'),('changed','bool')]}


def models():
    found = {}
    for module in MODULES:
        text = (ROOT / 'sdk/rust/src' / (module + '.rs')).read_text(encoding='utf-8')
        for name, body in re.findall(r'pub struct (\w+)\s*\{(.*?)\n\}', text, re.S):
            fields = re.findall(r'pub (\w+):\s*([^,\n]+)', body)
            if fields:
                found[name] = (module, fields)
    found.update({name: ('lib', fields) for name, fields in EXTRA.items()})
    return found


def ctype(t):
    t = t.strip()
    if t.startswith('Vec<'):
        return 'Slice<' + ctype(t[4:-1]) + '>'
    if t.startswith('Option<'):
        return 'Optional<' + ctype(t[7:-1]) + '>'
    if t == '(i32,i32,i32)':
        return 'Triple<i32,i32,i32>'
    return {'String':'Slice<u8>','bool':'u8','Color':'u32','NavItemKind':'u32','Path':'Slice<u16>'}.get(t, 'Ffi'+t if t[0].isupper() and t != 'Pixels' else t)


def qualified(t, model_map):
    if t.startswith('Vec<'):
        return 'Vec<' + qualified(t[4:-1], model_map) + '>'
    if t.startswith('Option<'):
        return 'Option<' + qualified(t[7:-1], model_map) + '>'
    if t in model_map:
        module = model_map[t][0]
        return ('crate::' if module=='lib' else f'crate::{module}::') + t
    return 'crate::NavItemKind' if t == 'NavItemKind' else t


def structure(name, fields):
    body = ['#[repr(C)]', '#[derive(Clone, Copy)]', 'pub struct '+name+' {']
    body.extend('    pub '+field+': '+ctype(t)+',' for field,t in fields)
    if not fields:
        body.append('    pub reserved: u8,')
    return '\n'.join(body+['}', ''])


def generate():
    model_map = models()
    model_lines = ['//! 自动生成的 C 数据视图；使用 tools/generate_runtime_ffi.py 更新。', 'use crate::{Slice, Optional, Pixels, Triple};', '']
    converters = ['// 自动生成的接口转换；不包含组件计算实现。', 'use slintforge_runtime_api::{Arena, Wire};', 'use slintforge_runtime_api::models::*;', '']
    for name, (module, fields) in model_map.items():
        model_lines.append(structure('Ffi'+name,fields))
        rust_name = qualified(name, model_map)
        converters += [f'impl Wire for {rust_name} {{', f'    type View = Ffi{name};', '    fn view(&self, arena: &mut Arena) -> Self::View {', f'        Ffi{name} {{']
        for field,t in fields:
            value = f'crate::wire_color(self.{field})' if t=='Color' else f'(self.{field} as u32)' if t=='NavItemKind' else f'self.{field}.view(arena)'
            converters.append(f'            {field}: {value},')
        converters += ['        }', '    }', '    unsafe fn read(value: Self::View) -> Self {', '        Self {']
        for field,t in fields:
            value = f'crate::wire_color_from(value.{field})' if t=='Color' else f'crate::wire_nav_kind(value.{field})' if t=='NavItemKind' else f'<{qualified(t,model_map)} as Wire>::read(value.{field})'
            converters.append(f'            {field}: {value},')
        converters += ['        }', '    }', '}', '']
    for index,(name,fields) in enumerate(OPS,1):
        model_lines += [f'pub const OP_{name.upper()}: u32 = {index};', structure('Args'+name,fields)]
    plot_source = (ROOT / 'sdk/common/ForgePlotCore.h').read_text(encoding='utf-8')
    import generate_plot_ffi
    plot_full = generate_plot_ffi.generate_native()
    plot_types, declarations = plot_full.split('extern "C" {',1)
    functions = []
    for name,args,ret in re.findall(r'pub fn (\w+)\((.*?)\)([^;]*);',declarations):
        functions.append((name,args,ret))
    table = ['#[repr(C)]', '#[derive(Clone, Copy)]', 'pub struct RuntimeApiV1 {', '    pub abi_version: u32,', '    pub struct_size: u32,', '    pub schema: [u8; 32],', '    pub call: unsafe extern "C" fn(u32, *const std::ffi::c_void, *mut CallResult) -> i32,', '    pub release: unsafe extern "C" fn(u64) -> i32,']
    for name,args,ret in functions:
        args = re.sub(r'\b(ForgePlot\w+)\b',r'plot::\1',args)
        ret = re.sub(r'\b(ForgePlot\w+)\b',r'plot::\1',ret)
        table.append(f'    pub {name}: unsafe extern "C" fn({args}){ret},')
    table += ['}', '']
    # 基础视图和所有权结构的变化也属于 ABI，不只检查生成的模型字段。
    base_contract = (ROOT / 'sdk/runtime-api/src/lib.rs').read_text(encoding='utf-8')
    layout = '\n'.join(re.findall(r'#\[repr\(C\)\].*?pub struct [^{]+\{[^}]*\}', base_contract, re.S))
    flow_contract = (ROOT / 'sdk/common/ForgeFlowCore.h').read_text(encoding='utf-8')
    flow_models = (ROOT / 'ui/types/flow.slint').read_text(encoding='utf-8')
    schema = hashlib.sha256(('\n'.join(model_lines)+plot_source+flow_contract+flow_models+'\n'.join(table)+layout).encode()).digest()
    table += ['pub const ABI_SCHEMA: [u8; 32] = '+repr(list(schema))+';', '']
    wrappers = ['//! 专业绘图公开接口的 DLL 转发；数据布局与共用头文件一致。', 'pub use slintforge_runtime_api::plot::*;', '']
    for name,args,ret in functions:
        names = ', '.join(arg.split(':',1)[0].strip() for arg in args.split(','))
        wrappers += [f'pub unsafe extern "C" fn {name}({args}){ret} {{', f'    (crate::runtime::api().{name})({names})', '}', '']
    files = {
        'sdk/runtime-api/src/models.rs': '\n'.join(model_lines),
        'sdk/runtime-api/src/plot.rs': plot_types,
        'sdk/runtime-api/src/table.rs': '\n'.join(table),
        'sdk/rust/src/wire.rs': '\n'.join(converters),
        'sdk/runtime/src/wire.rs': '\n'.join(converters),
        'sdk/rust/src/plot_ffi.rs': '\n'.join(wrappers),
        'sdk/runtime/src/plot_native.rs': 'pub use slintforge_runtime_api::plot::*;\nextern "C" {'+declarations,
        'sdk/runtime/src/api_table.rs': 'static API: RuntimeApiV1 = RuntimeApiV1 {\n    abi_version: ABI_VERSION,\n    struct_size: std::mem::size_of::<RuntimeApiV1>() as u32,\n    schema: ABI_SCHEMA,\n    call: runtime_call,\n    release: runtime_release,\n'+''.join(f'    {name}: plot_native::{name},\n' for name,_,_ in functions)+'};\n',
    }
    return {name: format_rust(text) for name,text in files.items()}


def format_rust(text):
    result = subprocess.run(['rustfmt','--edition','2021','--emit','stdout'],input=text,text=True,capture_output=True,cwd=ROOT,encoding='utf-8')
    if result.returncode:
        raise RuntimeError(result.stderr)
    return result.stdout


if __name__ == '__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--check',action='store_true')
    args=parser.parse_args()
    for name,text in generate().items():
        path=ROOT/name
        if args.check:
            if not path.is_file() or path.read_text(encoding='utf-8') != text:
                raise SystemExit('RUNTIME_FFI_STALE '+name)
        else:
            path.parent.mkdir(parents=True,exist_ok=True)
            path.write_text(text,encoding='utf-8')
    print('RUNTIME_FFI_OK')
