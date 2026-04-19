# Changes Since Last Presentation

*Please list all the updates, bug fixes, or enhancements made to the seminar content, report, or source code since the previous presentation.*

## Nguyễn Anh Kiệt (25125074)
- Since the presentation will be delivered via video, I don't see any immediate need for changes

## Trần Như Khải (25125045)
- Since the presentation will be delivered via video, I don't see any immediate need for changes

## Nguyễn Đức Huy (25125014)
- There have been no changes.

## Nguyễn Đình Minh Huy (25125083)
- Implemented DAG-focused data pipeline in source code:
	+ Added `SourceCode/ShortestPath/dag_graph_generator.cpp` and generated `dataset5_dag_sssp.txt` for DAG benchmarking.
	+ Updated `SourceCode/ShortestPath/graph_generator.cpp` so dataset headers are generated dynamically from output filenames.
- Extended benchmark system in source code:
	+ Updated `SourceCode/ShortestPath/read_graph_info.cpp` to support heap backend switching via `--heap-backend=custom|boost`.
	+ Added fair DAG benchmark path `dagShortestPath (precomputed topo order)` to reduce repeated topological-sort overhead during multi-source runs.
- Extended shortest-path implementation layer:
	+ Updated `SourceCode/ShortestPath/ShortestPath.h` and `SourceCode/ShortestPath/ShortestPath.cpp` with optional Boost-backed variants (`eagerDijkstraBoost`, `dijkstraWithBoostDaryHeap`) and runtime availability checks.
	+ Kept custom heap path as default fallback when Boost headers are unavailable.
- Regenerated benchmark results and logs for all datasets:
	+ Ran benchmarks for datasets 1-5 with both custom and boost heap backends.
	+ Added/updated logs such as `bench_dataset1_sparse_large_boost.log` through `bench_dataset5_dag_sssp_boost.log`.
- Report/documentation synchronization:
	+ Updated compile/run instructions and benchmark commands in `SeminarReport/content/sec07_source_codes.tex`.
	+ Updated dataset and interpretation details in `SeminarReport/content/sec08_datasets.tex`.