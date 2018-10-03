# 移植AWTK到SylixOS


## 一、获取源码

```
git clone https://github.com/zlgopen/awtk-sylixos.git
cd awtk-sylixos
git clone https://github.com/zlgopen/awtk.git

```

## 二、创建项目

> 所有路径请不要使用中文，避免不必要的麻烦。

* 1.用RealEvo-IDE创建SylixOS Standard Base项目，并命名为sylixos\_base。

* 2.用RealEvo-IDE创建SylixOS App项目，并命名为sylixos\_awtk，其base项目使用sylixos\_base。

* 3.拷贝awtk文件。
   * 请确保安装了python
   * 如果sylixos_awtk和sylixos_base不在awtk_sylixos目录下，请修改copy_files.py中的DST\_DEMOS\_ROOT和DST\_AWTK\_ROOT到正确目录。

```
python copy_files.py
```

## 三、配置项目sylixos_awtk(Properties->SylixOS Project->Compiler Settings)

* 0.先刷新一下sylixos_awtk项目

* 1.定义宏HAS\_AWTK\_CONFIG

* 2.包含下列路径
```
	sylixos_awtk/src/awtk/3rd/agge/include
	sylixos_awtk/src/awtk/3rd/libunibreak/src
	sylixos_awtk/src/awtk/3rd/agge/src
	sylixos_awtk/src/awtk/src/ext_widgets
	sylixos_awtk/src/awtk/demos
	sylixos_awtk/src/awtk
	sylixos_awtk/src/awtk/3rd/nanovg/src
	sylixos_awtk/src/awtk/3rd
	sylixos_awtk/src/awtk/3rd/stb
	sylixos_awtk/src/awtk-port
	sylixos_awtk/src/awtk/src
```

## 编译

编译sylixos_base和sylixos_awtk项目

## 运行和调试

* 1.上述项目缺省使用arm920t，所以请使用mini2440的虚拟机。
* 2.请先配置虚拟机的网络，虚拟机内部IP地址为192.168.7.30，主机端的IP地址可以设置为192.168.7.41。
* 3.创建设备192.168.7.30。
* 4.创建调试配置(SylixOS Remote Application)。

> 具体请看RealEvo-IDE的文档，以上只是几点提要。

## 移植

sylixos相关的代码在awtk-port目录里，如果要增加新的输入设备，请参考input\_thread.c。

## 已知问题

mini2440的鼠标坐标有点怪异：

* 1.左上角的坐标为(0, 936)，正常应该为(0, 0)。
* 2.右上角的坐标为(936, 936)，正常应该为(800, 0)。
* 3.右下角的坐标为(936, 0)，正常应该为(800, 480)。

所以搞了个不精确的换算公式：
```
    int x = mse_event.xmovement*info->max_x/936;
    int y = (936-mse_event.ymovement)*info->max_y/936;
```

有些地方的坐标仍然不够准确。

> 经确认是没有启用电阻屏校准，有空我再加上。

## 其它

* 1. 目前把资源直接编译到可执行文件中的，如果想把资源放到文件系统中。请在awtk_config.h中定义WITH\_FS\_RES，并将资源demos/assets放到可执行文件所在的目录。
* 2. 开发自己的程序时，把src/awtk/demos目录下的文件换成自己的，并修改copy\_files.py，避免文件被覆盖。
* 3. 目前没有加中文输入法，如需要可以自行加入(3rd/gpinyin)。









   
   
