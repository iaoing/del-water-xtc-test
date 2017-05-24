# change log

---

#### 2017-05-24

- 修改了 py-analysis/ana-waterNo.py 文件，添加了两个处理过程，分别为 caluDetType1 和 caluDetType2
- 生成了两个文件， caluDetType1.txt & caluDetType2.txt, 就是分别由上面提到的两个处理过程得到的
- caluDetType1.txt 更加理想化，但是效果不理想； caluDetType2.txt 效果更加好，但是开销一般般。
- **<u>*caluAvg.txt & caluPerWater.txt & detInfo.txt 因为之前处理有误，不再有用。*</u>**

---

#### 2017-05-23

- 添加了文件夹 origin-info, 以及 ./origin-info/waterNo.txt. 其中第一列、第二列、第三列是原始的数据，第四列和第五列 caluxxx 请无视。
- 添加了文件夹 py-analysis
  - ana-waterNo.py 分析 ./origin-info/waterNo.txt 中的数据
  - caluAvg.txt
  - caluPerWater.txt
  - detInfo.txt

---

#### 2017-05-22

- 在 plfs_xtc.h 中添加了一个函数 xtc_get_water_no(), 并在 plfs_xtc.c 中实现。

  - 该方法，可以通过解压坐标数据，获得 water atome 所在 buf 内的偏移量 offset




- 对 xtc_get_water_no() 进行了测试，测试代码在 test_get_waterNO.c 文件中