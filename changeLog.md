# change log

---

#### 2017-05-22

- 在 plfs_xtc.h 中添加了一个函数 xtc_get_water_no(), 并在 plfs_xtc.c 中实现。

  - 该方法，可以通过解压坐标数据，获得 water atome 所在 buf 内的偏移量 offset




- 对 xtc_get_water_no() 进行了测试，测试代码在 test_get_waterNO.c 文件中