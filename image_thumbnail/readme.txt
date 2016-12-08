1.	模块介绍
thumbnail_test是缩略图模块的测试程序，暂时支持JPEG/GIF/PNG格式的图片。

2.	命令参数介绍
Usage: thumbnail_test [-f file] [-w width] [-h height] [-q quality]  [-o file] [-help] [astv]
Option:
         -f file                	: the source file
         -a                     	: creat middle picture and thumbnail by default
         -w width               	: width of the thumbnail
         -h height              	: height of the thumbnail
         -q quality             	: quality of the thumbnail,default 60
         -s                     	: creat thumbnail for the file by size
         -o file                	: the output filename
         -t                     	: show time gap
         -v                    	: show version number
         -help          			: show help list

-f file: 添加原测试文件路径;
-a : 自动模式，自动生成中图(640x480)和缩略图(320x240)，名称为”middle_原文件名”和”small_原文件名”;
-s : 手动模式，生成手动输入的格式的缩略图图片；
-w : 手动模式，指定图片宽度；
-h : 手动模式，指定图片高度；
-q : 手动模式，指定图片质量（0-100）；
-o : 手动模式，指定生成图片路径；
-t : 打印命令执行时间；
-v : 打印模块名称和版本号；
-help: 打印帮助； 

3.	例子
1）thumbnail_test -a -f 1.jpeg
自动生成middle_1.jpeg和small_1.jpeg的中图和缩略图；
2）thumbnail_test -s -w 1000 -h 1000 -q 90 -f 1.jpeg -o a.jpeg
按照宽为1000、高为1000（最终生成的高宽要根据原图的长宽比例）、质量为90的格式生成a.jpeg图片。

注：GIF的中图和缩略图都是取原图的第一帧；-q 参数对GIF和PNG无效。
