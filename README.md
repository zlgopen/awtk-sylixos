# AWTK针对sylixos平台的移植。

[AWTK](https://github.com/zlgopen/awtk)是为嵌入式系统开发的GUI引擎库。

[awtk-sylixos](https://github.com/zlgopen/awtk-sylixos)是AWTK在sylixos上的移植。

## 获取源码

```
git clone https://github.com/zlgopen/awtk-sylixos.git
cd awtk-sylixos
git clone https://github.com/zlgopen/awtk.git

```

## 编译

在RealEvo-IDE中导入sylixos-base/sylixos-awtk两个项目并编译。

## 运行和调试(具体请看RealEvo-IDE的文档)

* 缺省使用arm920t，所以请使用mini2440的虚拟机。


## 同步最新的AWTK源码

```
python copy_files.py
```

以上方法只是评估时看看awtk的demos效果，真正使用时请参考[移植AWTK到SylixOS](doc/guide.md)
