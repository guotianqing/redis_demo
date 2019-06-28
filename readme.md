### 简介
---

项目中使用到了redis，使用c++语言，了解了相关的库的使用。

主要针对hiredis库，c/c++都可以使用，安装简单，使用方便，类printf语法。

也增加了对cppredis的demo，详情可以参考github仓库。

可以以自己需要的方式把数据从redis读取到文件，并可以自由格式化。

### 依赖
---

请自行安装redis, hiredis和cppredis库。

编译使用scons组件，请安装。它类似于Makefile，但基于python语法，更简洁。
关于scons的更多介绍，请参考：[Scons入门简介](https://blog.csdn.net/guotianqing/article/details/92003258)

### 编译
---

运行scons命令即可。
可通过修改SConstruct文件修改具体编译命令。
