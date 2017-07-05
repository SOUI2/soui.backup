###                                       SouiEdiotr界面编辑器使用说明

准备工作, 编译SouiEditor后, 需要将SouiEditor目录的**Config目录拷贝到SouiEditor可执行文件**所在目录.

运行后主界面如下图:

![img](https://github.com/SOUI2/soui/raw/snow/souieditor/demos/SouiEditor/docimage/1.png)

 

主界面上, **Layout**标签页中列出所有窗体的XML文件, **XML文件** 列表中是除了窗体文件外的其它XML文件(如string, color 等的定义文件)。

在主界面中间, 切换到窗体标签页时, 双击Layout下的窗体文件, 将进入可视化编辑窗体. 

如果是在XML编辑标签页激活时, 双击Layout下的窗体文件, 则直接以文件的方式打开窗体文件进行编辑(这时就相当于一个普通的文本编辑器, 修改后按Ctrl+S保存文件)

除窗体文件外的其它XML文件, 双击后在**XML编辑标签页** 打开，以文本方式编辑

 

在可视化编辑模式时，在窗体可视编辑窗口中点击控件，代码窗口会自动定位此控件代码。演示如下

![img](https://github.com/SOUI2/soui/raw/snow/souieditor/demos/SouiEditor/docimage/1.gif)





**SouiEditor也支持以命令行方式打开工程, 如VS中可设置如下, 方便随时编辑**.

![img](https://github.com/SOUI2/soui/raw/snow/souieditor/demos/SouiEditor/docimage/2.png)

 

编辑器的代码编辑窗口提供了自动完成功能，省去了记忆很多控件名与属性等。

**目前支持的自动完成功能如下。**

1、 增加控件提示

>  按<后将出现所有控件的列表

2、 属性提示

>  在任意控件范围内，输入任意属性则会进行提示

3、 自定义skin提示

>  如在skin=” 按 . 将出现所有已定义的skin列表

4、 自定义class提示

>  如在class=” 按 / 将出现所有已定义的style列表

5、 自定义string提示

>  @string/ 在已输入@string后按下/将出现所有已定义的字符串列表

6、 自定义color提示

>  @color/ 在已输入@ color后按下/将出现所有已定义的颜色列表



自动提示功能的演示参见下图：

![img](https://github.com/SOUI2/soui/raw/snow/souieditor/demos/SouiEditor/docimage/3.gif)