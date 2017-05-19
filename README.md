# del-water-xtc-test

Delete water molecular in xtc file that could reduce it's size. This is only a test, not a prototype.

- plfs_pdb
  - 重载了有关于 .pdb 文件的函数
- plfs_xtc
  - 重载了有关于 .xtc 文件的函数
- test_all.c
  - 测试了读取 .pdb 文件确定 water atome 位置，将其置为0，之后写回到一个新的 .xtc 文件中，并重新读取验证争取性的过程
  - 可以使用 vmd 正确加载，并正确显示
  - 此测试通过解压，将在置 water atome 为0的
- test_pdb.c
  - 测试了读取 .pdb 文件并找到 water atome 位置的过程
- test_xtc.c
  - 测试了，单独读取 .xtc 文件，随机置位0，之后写回到一个新 .xtc 文件，并重新读取验证争取性的过程
  - 可以使用 vmd 正确加载，并正确显示
- test_split_xtc_by_uncompressed.c
  - 测试读取 .pdb 文件确定 water atome 位置，并根据此信息 split .xtc 文件，使其 split 为不含有 water atome 的 .xtc 文件
  - 此测试通过解压，将在置 water atome 为0的，所以精准度十足   
- test_split_xtc_by_not_uncompressed.c
  - 测试读取 .pdb 文件确定 water atome 位置，并根据此信息 split .xtc 文件，使其 split 为不含有 water atome 的 .xtc 文件
  - 因为是通过获取的 water atome 的入界 index 和出界 index 信息分割的，所以目前存在**<u>不精准</u>**的问题
- test_get_bytecnt_bitsize.c
  - 测试读取 .xtc 文件，确定每一帧的 bytes count 和 bitsize ——指导的每个 int(x/y/z-coord) 或者每组三个 int(x,y,z-coord) 的填充大小。因为只是指导性的，实际上比这个指导性的意见小很多

