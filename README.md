# AWTK针对sylixos平台的移植。

[AWTK](https://github.com/zlgopen/awtk)是为嵌入式系统开发的GUI引擎库。

[awtk-sylixos](https://github.com/zlgopen/awtk-sylixos)是AWTK在sylixos上的移植。

## 使用方法

* 1.获取源码

```
git clone https://github.com/zlgopen/awtk-sylixos.git
cd awtk-sylixos
git clone https://github.com/zlgopen/awtk.git

```

* 2.在RealEvo-IDE中导入sylixos-base/sylixos-awtk两个项目。

## 同步最新的AWTK源码

* 1.修改copy_files.py中的DST_ROOT为正确路径(不支持中文路径)

```
DST_ROOT='C:\\work\\awtk-sylixos\\sylixos_awtk\\src';
```

* 2.运行脚本拷贝文件

```
python copy_files.py
```

