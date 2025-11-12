# 3D数据管理与可视化系统 (3D Data Visualization System)

> 基于 C++17 + Qt6 + OpenGL 的轻量级点云 / 网格模型导入、分析与可视化交互程序。支持模型多实例管理、几何统计、伪彩色渲染与基础变换。

## 🌟 主要特性总览

| 分类 | 能力 | 说明 |
|------|------|------|
| 数据类型 | PointCloud / Mesh | 基于抽象基类 `Model`，统一属性与接口 |
| 导入格式 | PLY / OBJ / XYZ | 简化解析；PLY 当前仅支持 ASCII 顶点+可选颜色；OBJ 多边形扇形三角化；XYZ 纯坐标 |
| 导出格式 | PLY / OBJ / XYZ | 统一使用当前模型顶点（含颜色），OBJ 法线按顶点法线导出 |
| 单位管理 | 导入单位选择 (m/cm/mm) | 内部统一用米存储；界面显示和伪彩色使用厘米；表面积以 cm² 输出 |
| 可视化 | 固定管线 OpenGL | 支持坐标轴、网格、包围盒高亮、伪彩色映射与 RGB 手动颜色 |
| 伪彩色 | Rainbow / Viridis / Red-Blue | 基于选定轴 X/Y/Z 的全局最值范围映射 t∈[0,1] |
| 几何分析 | 重心 (cm) / AABB (cm) / 表面积 (cm²) | 表面积仅 Mesh；点云三角片统计 |
| 交互 | 旋转 / 平移 / 缩放视角 | 鼠标左旋转、右平移、滚轮缩放相机；模型位置通过数值平移到重心 |
| 多模型 | 添加测试数据 / 导入 / 删除 / 全部清除 | 模型列表支持选择，高亮包围盒 |
| 性能策略 | 简单遍历 / 每帧范围预计算 | 每帧预扫描轴最值，减少逐顶点重复统计 |

## 🧩 架构与代码组织

核心采用“数据模型 + OpenGLWidget 渲染 + 主窗口 UI”三层：

- `Model` 抽象基类：名称、颜色、顶点数组、三角形索引、单位换算辅助（重心 / AABB 以米内部存储 → 输出换算为厘米）。
- `PointCloud`：缓存统计（lazy：脏标记 + 计算重心与 AABB）。
- `Mesh`：提供三角面片添加与表面积计算（面片面积 m² → cm²）。
- `OpenGLWidget`：统一相机、坐标轴/网格、伪彩色与包围盒绘制，固定管线实现；帧内预计算伪彩色轴范围避免 O(N*M) 重复遍历。
- `FileImporter`：格式判定 + 简化解析 + 三角化（OBJ 与含面 PLY）。
- `ModelAnalyzer`：生成几何统计文本（用于信息面板）。
- `TransformTool`：提供通用向量 / 批量平移、旋转（Rodrigues）、缩放与组合矩阵。
- `ColorMapper`：基础 HSV 伪彩色映射函数（部分逻辑已内嵌渲染实现中，保留工具类便于拓展）。

### 目录结构（精简）
```
include/
  MainWindow.h        # 主窗口
  OpenGLWidget.h      # 渲染部件
  Model.h / PointCloud.h / Mesh.h
  Vertex.h / AABB.h   # 基础数据结构
  FileImporter.h      # 导入导出
  ModelAnalyzer.h     # 几何统计
  TransformTool.h     # 变换算法
  ColorMapper.h       # 颜色映射工具
src/
  main.cpp            # 程序入口
  MainWindow.cpp      # UI 搭建与交互
  OpenGLWidget.cpp    # 渲染 & 交互逻辑
  Model.cpp / PointCloud.cpp / Mesh.cpp
  FileImporter.cpp / ModelAnalyzer.cpp
  TransformTool.cpp / ColorMapper.cpp
CMakeLists.txt         # CMake 构建配置
3DDataVisualization.pro# Qt .pro（可选）
run.bat                # Windows 运行脚本（设置 Qt DLL 路径）
```

## 🛠️ 构建与运行

### 环境要求
1. CMake >= 3.16
2. Qt6（需包含 Core / Widgets / OpenGL / OpenGLWidgets 模块）
3. 支持 OpenGL 的 GPU 与驱动（固定管线兼容性良好）
4. C++17 编译器（MinGW / MSVC 均可）

### 使用 CMake 手动构建（PowerShell）
```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..    # 或 -G "NMake Makefiles" / Ninja
cmake --build . --config Release
./3DDataVisualization.exe         # 或在资源管理器双击
```

### 使用 Qt Creator
1. 打开 `CMakeLists.txt`
2. 选择 Qt6 Kit（含 OpenGLWidgets）
3. 构建并运行

### 使用现有脚本 `run.bat`
脚本会：
- 预置 Qt DLL 路径（确保路径与本地安装匹配）
- 进入 `build/` 并启动 `3DDataVisualization.exe`
若 DLL 未找到：请调整 `run.bat` 中的 Qt 安装路径。

## 📦 模型导入 / 导出说明

| 格式 | 导入支持 | 导出支持 | 当前限制 |
|------|----------|----------|----------|
| PLY  | ASCII 顶点 + 可选 RGB；若含面则视为 Mesh | 顶点+颜色，不写面数据（当前不区分是否 Mesh） | 二进制仅尝试按点云读取；未解析法线/面属性扩展 |
| OBJ  | 顶点 (v) + 面 (f)，多边形扇形三角化 | 顶点 (v) + 法线 (vn) + 三角面 (f) | 纹理坐标忽略；法线导入未与面关联；材质未支持 |
| XYZ  | 每行 x y z | 顶点坐标 | 无颜色、法线与面信息 |

导入单位：通过右侧“导入单位”下拉选择 m / cm / mm，会对读取的几何整体进行倍率缩放（内部存储仍为米）。显示与伪彩色统一使用厘米。

## 🎨 可视化与交互

| 操作 | 说明 |
|------|------|
| 左键拖拽 | 旋转相机（Yaw / Pitch）|
| 右键拖拽 | 平移相机目标点 |
| 滚轮 | 缩放（调整相机距离）|
| 模型列表选择 | 高亮对应 AABB（黄色线框）|
| 重置相机 | 菜单“视图”->“重置相机”|
| 清除全部模型 | 菜单“视图”->“清除所有模型”|

伪彩色：
1. 勾选“伪彩色渲染”。
2. 选择坐标轴（X/Y/Z）。
3. 每帧自动计算所选轴全局 min/max（厘米）。
4. 根据 t=(coord-min)/(max-min) 映射到选定色图：
   - Rainbow：蓝→青→绿→黄→红（简化 HSV）
   - Viridis：近似分段插值实现（深紫→黄）
   - Red-Blue：蓝→白→红 双端梯度

颜色与包围盒：
- 未启用伪彩色时：使用顶点固有颜色或模型统一颜色。
- 选中模型：绘制 AABB 黄框。

## 📊 几何与单位换算

- 内部顶点单位：米 (m)
- UI 显示：重心 / AABB / 表面积均换算到厘米 / 平方厘米。
- 表面积计算：三角形叉积 0.5×|cross(e1,e2)|（m²）→ ×10000 转为 cm²。
- 平移输入框：按重心位置（厘米）进行偏移换算，调用 `translateCm()`。

## 🧪 核心类速览

| 类 | 作用 | 要点 |
|----|------|------|
| Model | 抽象基类 | 顶点/索引、颜色、平移/旋转/缩放、重心/AABB 计算（重心输出 cm）|
| PointCloud | 点云模型 | Lazy 缓存统计；无面片；继承变换接口 |
| Mesh | 网格模型 | 三角面管理；表面积计算；面片添加与三角化 |
| OpenGLWidget | 场景渲染 | 相机控制、伪彩色、坐标轴/网格、包围盒、固定管线 |
| FileImporter | 文件 IO | 格式检测 + 简化解析 + 导出统一格式 |
| ModelAnalyzer | 信息统计 | 文本化输出（重心 cm / AABB cm / 面面积）|
| TransformTool | 几何变换 | 向量批量运算 + Rodrigues 旋转矩阵生成 |
| ColorMapper | 颜色工具 | HSV 转换与高度/距离示例映射（当前主要在渲染内实现自定义色图）|

## ⚠️ 当前限制与注意事项

1. 使用固定管线 OpenGL（未使用现代可编程着色器）。
2. 未做法线重建与平滑（Mesh 立方体示例统一法线）。
3. OBJ 纹理坐标、材质、法线索引未完整支持。
4. PLY 二进制解析仅做基本兼容提示，不保证所有变体有效。
5. 没有撤销 / 重做栈（README 旧描述中的撤销功能暂未实现）。
6. 没有多线程与异步 IO，超大数据将导致 UI 卡顿。
7. 点云渲染使用逐点立即模式，超大规模（>百万点）性能有限。
8. 导出 PLY 当前不包含面片（若需保留 Mesh 面片需扩展写入 `element face` 部分）。
9. 重心用于定位 UI 位置控制，实际模型没有独立世界矩阵（直接修改顶点坐标）。

## 🚀 后续可拓展方向

- 引入现代 OpenGL (VAO/VBO + GLSL) & Instancing / SSBO 优化性能
- 增加 STL / LAS / PCD / FBX 等更多格式支持
- 引入法线重建（PCA / 邻域估计）与光照高级材质
- 大数据点云分块加载 / 八叉树裁剪 / GPU 点大小动态缩放
- 增加局部编辑（顶点/面选择、删除、合并）与简易网格修复
- 引入多线程解析与进度反馈
- 添加单元测试框架（如 Catch2 / GoogleTest）验证几何计算正确性

## ❓ 常见问题 (FAQ)

| 问题 | 可能原因 | 解决方案 |
|------|----------|----------|
| 程序启动报缺少 Qt DLL | `run.bat` 路径与本地 Qt 安装不一致 | 修改脚本 Qt 安装路径或使用 windeployqt |
| 导入 OBJ 面片错乱 | 模型包含四边以上多边形 | 当前使用扇形三角化，复杂拓扑需预处理 |
| 表面积显示为 0 | 选择的是点云模型 | 点云无三角面；仅 Mesh 显示面积 |
| 伪彩色全为单色 | 模型坐标范围极小或全部相同 | 数据归一化后 range≈0，被强制设为 1；确认数据是否含变化 |
| 修改位置后重心不符预期 | 顶点直接被平移 | 没有矩阵层级；平移即修改所有顶点坐标 |
| Viridis 色图不够平滑 | 近似插值实现 | 可用查表或正式渐变控制点细化 |


## 📝 快速开始 Checklist

1. 安装 Qt6 与 CMake。
2. `git clone` 项目后进入根目录。
3. 执行构建命令或用 Qt Creator 打开。
4. 运行程序，导入示例点云或点击“添加测试点云”。
5. 试用伪彩色与不同色图模式。

祝你使用愉快！如需新增功能，可在 ISSUE / TODO 中追加需求。
