#!/usr/bin/bash

echo Storm-VWT version 0.0.1 自动执行模块
cd   Wind
echo 正在分配速率...
./speed
rm   ../Storm-VWT/vxyz_init.wvp ../Storm-VWT/vxyz_RT.wvr
echo 正在移动文件...
mv   vxyz_init.wvp ../Storm-VWT/vxyz_init.wvp
mv   vxyz_RT.wvr   ../Storm-VWT/vxyz_RT.wvr
cd   Storm-VWT
storm_vwt -n 2012 -a -5.0 -A 45.0
storm_vwt -n 2112 -a -5.0 -A 45.0
storm_vwt -n 2212 -a -5.0 -A 45.0
storm_vwt -n 2312 -a -5.0 -A 45.0
storm_vwt -n 2412 -a -5.0 -A 45.0
cd   Wind
echo 正在分配速率...
./speed
rm   ../Storm-VWT/vxyz_init.wvp ../Storm-VWT/vxyz_RT.wvr
echo 正在移动文件...
mv   vxyz_init.wvp ../Storm-VWT/vxyz_init.wvp
mv   vxyz_RT.wvr   ../Storm-VWT/vxyz_RT.wvr