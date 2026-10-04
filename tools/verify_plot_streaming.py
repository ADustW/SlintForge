"""检查增量帧性能、历史连续性记录、截图和本轮源码证据。"""
from pathlib import Path
import argparse
import csv
import hashlib
import json
import statistics
import xml.etree.ElementTree as ET
from PIL import Image, ImageStat

ROOT=Path(__file__).resolve().parents[1]
def percentile(values,p):
    values=sorted(values)
    return values[min(len(values)-1,int((len(values)-1)*p))]
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--directory',default='build/plot-streaming-qa');args=parser.parse_args();directory=ROOT/args.directory
    report={'hosts':{},'core':{},'hashes':{}}
    for host in ('rust','cpp'):
        folder=directory/f'{host}-window'
        frames=list(csv.DictReader((folder/'incremental-frame-performance.csv').open(encoding='utf-8')))
        assert len(frames)==64,(host,len(frames))
        for index,frame in enumerate(frames):
            assert int(frame['history_points'])==1000000+(index+1)*32,frame
            assert int(frame['rendered_points'])<5000,frame
        times=[float(r['total_ms']) for r in frames];append=[float(r['append_ms']) for r in frames]
        assert percentile(times,.95)<50,(host,'完整帧 p95',percentile(times,.95))
        assert percentile(append,.95)<5,(host,'追加 p95',percentile(append,.95))
        continuous=list(csv.DictReader((folder/'streaming-window-performance.csv').open(encoding='utf-8')))
        assert len(continuous)>=50,(host,len(continuous))
        counts=[int(row['history_points']) for row in continuous]
        assert counts==sorted(counts) and counts[-1]>4000,(host,counts)
        expected=[f'legend-{p}-{s}.png' for p in range(8) for s in range(3)]
        expected += [f'line-style-{i}.png' for i in range(6)] + [f'axis-ticker-{i}.png' for i in range(5)] + ['history-measure.png','live-before-pause.png']
        for name in expected:
            with Image.open(folder/name) as image:
                image.load();assert image.size==(1080,760),(host,name,image.size)
                assert max(ImageStat.Stat(image.convert('RGB')).stddev)>15,(host,name,'空白截图')
        report['hosts'][host]={'frames':len(frames),'frame_median_ms':statistics.median(times),'frame_p95_ms':percentile(times,.95),'append_median_ms':statistics.median(append),'append_p95_ms':percentile(append,.95),'continuous_frames':len(continuous),'continuous_history_points':counts[-1],'screenshots':len(expected)}
    rows=list(csv.DictReader((directory/'core/streaming-performance.csv').open(encoding='utf-8')))
    times=[float(r['render_ms']) for r in rows]
    assert len(rows)==100 and int(rows[-1]['history_points'])==1003200
    for file in (directory/'core').glob('legend-*.svg'):ET.parse(file)
    report['core']={'history_points':1003201,'frames':len(rows),'window_render_median_ms':statistics.median(times),'window_render_p95_ms':percentile(times,.95)}
    files=['sdk/common/ForgePlotCore.cpp','sdk/common/ForgePlotCore.h','sdk/rust/src/plot.rs','sdk/rust/src/plot_ffi.rs','sdk/rust/src/plot_bindings.rs','sdk/cpp/include/ForgePlot.hpp','sdk/cpp/include/ForgePlotBindings.hpp','ui/types/plot.slint','ui/charts/plot.slint','ui/slintforge.slint','examples/ui/professional-plot-demo.slint','examples/ui/professional-plot.slint','Gallery/ui/component/plot-page.slint','Gallery/ui/component/chart-pages.slint','Gallery/ui/screen/main-screen.slint','Gallery/rust/src/plot_demo.rs','Gallery/cpp/ForgeGalleryPlot.cpp','examples/plot-rust/src/main.rs','examples/plot-rust/src/streaming_qa.rs','examples/plot-cpp/main.cpp','examples/plot-cpp/streaming_qa.hpp','tests/cpp/ForgePlotTests.cpp','Gallery/rust/tests/plot.rs','Gallery/data/navigation.txt','docs/PLOT.md','docs/PLOT_STREAMING_IMPLEMENTATION.md','target/debug/slintforge-gallery-rust.exe','target/debug/slintforge-plot-example.exe','build/cpp/Gallery/cpp/Debug/slintforge_gallery_cpp.exe','build/plot-streaming-qa/cpp-build/Release/slintforge_plot_example.exe']
    for name in files:report['hashes'][name]=hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
    (directory/'evidence.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps(report['hosts'],ensure_ascii=False));print('PLOT_STREAMING_QA_OK')
if __name__=='__main__':main()
