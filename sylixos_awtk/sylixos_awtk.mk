#*********************************************************************************************************
#
#                                    中国软件开源组织
#
#                                   嵌入式实时操作系统
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: sylixos_awtk.mk
#
# 创   建   人: RealEvo-IDE
#
# 文件创建日期: 2018 年 10 月 03 日
#
# 描        述: 本文件由 RealEvo-IDE 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := sylixos_awtk

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS :=  \
src/awtk/3rd/agge/src/agge/math.cpp \
src/awtk/3rd/agge/src/agge/nanovg_agge.cpp \
src/awtk/3rd/agge/src/agge/nanovg_vertex.cpp \
src/awtk/3rd/agge/src/agge/stroke.cpp \
src/awtk/3rd/agge/src/agge/stroke_features.cpp \
src/awtk/3rd/agge/src/agge/vector_rasterizer.cpp \
src/awtk/3rd/libunibreak/src/graphemebreak.c \
src/awtk/3rd/libunibreak/src/graphemebreakdata.c \
src/awtk/3rd/libunibreak/src/linebreak.c \
src/awtk/3rd/libunibreak/src/linebreakdata.c \
src/awtk/3rd/libunibreak/src/linebreakdef.c \
src/awtk/3rd/libunibreak/src/unibreakbase.c \
src/awtk/3rd/libunibreak/src/unibreakdef.c \
src/awtk/3rd/libunibreak/src/wordbreak.c \
src/awtk/3rd/libunibreak/src/wordbreakdata.c \
src/awtk/3rd/nanovg/src/nanovg.c \
src/awtk/demos/assets.c \
src/awtk/demos/demo_main.c \
src/awtk/demos/demo_ui_app.c \
src/awtk/src/awtk.c \
src/awtk/src/base/app_bar.c \
src/awtk/src/base/array.c \
src/awtk/src/base/assets_manager.c \
src/awtk/src/base/bitmap.c \
src/awtk/src/base/buffer.c \
src/awtk/src/base/button.c \
src/awtk/src/base/button_group.c \
src/awtk/src/base/canvas.c \
src/awtk/src/base/check_button.c \
src/awtk/src/base/color.c \
src/awtk/src/base/color_parser.c \
src/awtk/src/base/color_tile.c \
src/awtk/src/base/column.c \
src/awtk/src/base/combo_box.c \
src/awtk/src/base/combo_box_item.c \
src/awtk/src/base/custom_props.c \
src/awtk/src/base/dialog.c \
src/awtk/src/base/dialog_client.c \
src/awtk/src/base/dialog_title.c \
src/awtk/src/base/dragger.c \
src/awtk/src/base/easing.c \
src/awtk/src/base/edit.c \
src/awtk/src/base/emitter.c \
src/awtk/src/base/enums.c \
src/awtk/src/base/events.c \
src/awtk/src/base/event_queue.c \
src/awtk/src/base/font.c \
src/awtk/src/base/font_manager.c \
src/awtk/src/base/fs.c \
src/awtk/src/base/glyph_cache.c \
src/awtk/src/base/grid.c \
src/awtk/src/base/group_box.c \
src/awtk/src/base/idle.c \
src/awtk/src/base/image.c \
src/awtk/src/base/image_loader.c \
src/awtk/src/base/image_manager.c \
src/awtk/src/base/input_engine.c \
src/awtk/src/base/input_method.c \
src/awtk/src/base/label.c \
src/awtk/src/base/layout.c \
src/awtk/src/base/lcd.c \
src/awtk/src/base/line_break.c \
src/awtk/src/base/locale_info.c \
src/awtk/src/base/main_loop.c \
src/awtk/src/base/matrix.c \
src/awtk/src/base/mem.c \
src/awtk/src/base/pages.c \
src/awtk/src/base/path.c \
src/awtk/src/base/popup.c \
src/awtk/src/base/progress_bar.c \
src/awtk/src/base/rect.c \
src/awtk/src/base/rom_fs.c \
src/awtk/src/base/row.c \
src/awtk/src/base/slider.c \
src/awtk/src/base/spin_box.c \
src/awtk/src/base/str.c \
src/awtk/src/base/suggest_words.c \
src/awtk/src/base/system_info.c \
src/awtk/src/base/tab_button.c \
src/awtk/src/base/tab_button_group.c \
src/awtk/src/base/tab_control.c \
src/awtk/src/base/theme.c \
src/awtk/src/base/time.c \
src/awtk/src/base/timer.c \
src/awtk/src/base/tokenizer.c \
src/awtk/src/base/ui_builder.c \
src/awtk/src/base/utf8.c \
src/awtk/src/base/utils.c \
src/awtk/src/base/value.c \
src/awtk/src/base/velocity.c \
src/awtk/src/base/vgcanvas.c \
src/awtk/src/base/view.c \
src/awtk/src/base/widget.c \
src/awtk/src/base/widget_animator.c \
src/awtk/src/base/widget_factory.c \
src/awtk/src/base/widget_vtable.c \
src/awtk/src/base/window.c \
src/awtk/src/base/window_animator.c \
src/awtk/src/base/window_manager.c \
src/awtk/src/base/wstr.c \
src/awtk/src/blend/blend_image_565_565.c \
src/awtk/src/blend/blend_image_565_8888.c \
src/awtk/src/blend/blend_image_8888_565.c \
src/awtk/src/blend/blend_image_8888_8888.c \
src/awtk/src/blend/image_g2d.c \
src/awtk/src/blend/soft_g2d.c \
src/awtk/src/blend/stm32_g2d.c \
src/awtk/src/ext_widgets/color_picker/color_component.c \
src/awtk/src/ext_widgets/color_picker/color_picker.c \
src/awtk/src/ext_widgets/color_picker/rgb_and_hsv.c \
src/awtk/src/ext_widgets/ext_widgets.c \
src/awtk/src/ext_widgets/guage/guage.c \
src/awtk/src/ext_widgets/image_animation/image_animation.c \
src/awtk/src/ext_widgets/keyboard/candidates.c \
src/awtk/src/ext_widgets/keyboard/keyboard.c \
src/awtk/src/ext_widgets/rich_text/rich_text.c \
src/awtk/src/ext_widgets/rich_text/rich_text_node.c \
src/awtk/src/ext_widgets/rich_text/rich_text_parser.c \
src/awtk/src/ext_widgets/rich_text/rich_text_render_node.c \
src/awtk/src/ext_widgets/scroll_view/list_item.c \
src/awtk/src/ext_widgets/scroll_view/list_view.c \
src/awtk/src/ext_widgets/scroll_view/list_view_h.c \
src/awtk/src/ext_widgets/scroll_view/scroll_bar.c \
src/awtk/src/ext_widgets/scroll_view/scroll_view.c \
src/awtk/src/ext_widgets/slide_view/slide_view.c \
src/awtk/src/ext_widgets/switch/switch.c \
src/awtk/src/ext_widgets/text_selector/text_selector.c \
src/awtk/src/ext_widgets/time_clock/time_clock.c \
src/awtk/src/font/font_bitmap.c \
src/awtk/src/font/font_stb.c \
src/awtk/src/image_loader/image_loader_stb.c \
src/awtk/src/input_engines/input_engine_null.c \
src/awtk/src/input_methods/input_method_creator.c \
src/awtk/src/lcd/lcd_mem_bgra8888.c \
src/awtk/src/lcd/lcd_mem_rgb565.c \
src/awtk/src/lcd/lcd_mem_rgba8888.c \
src/awtk/src/main_loop/main_loop_simple.c \
src/awtk/src/misc/new.cpp \
src/awtk/src/misc/test_cpp.cpp \
src/awtk/src/platforms/pc/fs_os.c \
src/awtk/src/platforms/pc/mutex.c \
src/awtk/src/platforms/pc/platform.c \
src/awtk/src/platforms/pc/thread.c \
src/awtk/src/ui_loader/ui_binary_writer.c \
src/awtk/src/ui_loader/ui_builder_default.c \
src/awtk/src/ui_loader/ui_loader.c \
src/awtk/src/ui_loader/ui_loader_default.c \
src/awtk/src/ui_loader/ui_loader_xml.c \
src/awtk/src/ui_loader/ui_serializer.c \
src/awtk/src/ui_loader/ui_xml_writer.c \
src/awtk/src/ui_loader/window_open.c \
src/awtk/src/vgcanvas/vgcanvas_nanovg.c \
src/awtk/src/widget_animators/widget_animator_move.c \
src/awtk/src/widget_animators/widget_animator_opacity.c \
src/awtk/src/widget_animators/widget_animator_rotation.c \
src/awtk/src/widget_animators/widget_animator_scale.c \
src/awtk/src/widget_animators/widget_animator_scroll.c \
src/awtk/src/widget_animators/widget_animator_value.c \
src/awtk/src/window_animators/window_animator_fb.c \
src/awtk/src/xml/xml_builder.c \
src/awtk/src/xml/xml_parser.c \
src/awtk-port/input_dispatcher.c \
src/awtk-port/input_thread.c \
src/awtk-port/lcd_sylixos_fb.c \
src/awtk-port/main_loop_sylixos.c \
src/sylixos_awtk.c

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH :=  \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk-port" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/demos" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/src" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/src/ext_widgets" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd/agge/include" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd/agge/src" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd/libunibreak/src" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd/nanovg/src" \
-I"$(WORKSPACE_sylixos_awtk)/src/awtk/3rd/stb"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL :=  \
-DHAS_AWTK_CONFIG=1

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS := 
LOCAL_CXXFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB := 
LOCAL_DEPEND_LIB_PATH := 

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := no
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# OpenMP config
#*********************************************************************************************************
LOCAL_USE_OMP := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD := 
LOCAL_POST_LINK_CMD := 
LOCAL_PRE_STRIP_CMD := 
LOCAL_POST_STRIP_CMD := 

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := 

include $(APPLICATION_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
