C++ Xml解析的效率比较（Qt/TinyXml2/RapidXml/PugiXml）
通常我们在一些软件的初始化或者保存配置时都会遇到对XML文件的操作，包括读写xml文件，解析内容等等。在我的工作中就遇到了这么一个问题，就是在ARM平台下Qt解析xml文件非常的慢，最初怀疑是我的操作有问题或者是ARM平台下的文件操作本身就很慢，于是就开始调查到底是哪里的效率问题，下面是一些测试分享给大家。

问题背景
下面一段代码是前面提到的运行效率低的一段代码：

QString filename = "...";
QFile file( filename );

//< step1 open file
if( !file.open(QIODevice::ReadOnly) )
{
    qDebug() << "failed in opening file!";
    return false;
}

//< step2 read file content
QDomDocument doc;       //< #include <qdom.h>
if( !doc.setContent( &file ) )
{
    qDebug() << "failed in setting content!";
    file.close();
    return false;
}
file.close();
...  //< operations on the content of file!

起初以为是文件打开和关闭耗时太多，所以在文件open和close函数前后都获取了系统时间来测试了函数消耗时间，结果是耗时很短，反而是 doc.setContent 耗费了非常长的时间，这才发现原来是Qt获取XML文件内容且Dom模型结构花费了太多时间，所以我们开始寻求效率更高的解决方案。

测试环境
Windows： 
system：windows 10 
cpu: intel core-i5-5200u @2.2GHz 
IDE: visual studio 2010 
compiler: VC10
Linux: 
system: Debian 4.4.5-8 
cpu: intel core-i5-3450 @3.3GHz 
IDE: VIM 
compiler: gcc version 4.4.5
Qt版本： 4.8.4
用来测试的文件名为 DriverConfig.xml，大小为245Kb，共1561行，大部分内容为中文
比较项有 TinyXml2， QDomDocument，因为从接口来看这两者的操作方式很类似，后面我会加入其它的xml解析库的比较，如 xmlbooster 等。

Qt - QDomDocument

下面是利用Qt中的xml支持来读取文件内容的源代码：
#include <QtCore/QCoreApplication>
#include <qdom.h>
#include <QFile>
#include <QIODevice>
#include <iostream>
#ifdef Q_OS_WIN
# include <Windows.h>
#else
# include <sys/time.h>
#endif

using std::cout;
using std::endl;

#define TEST_TIMES 10

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef Q_OS_WIN  //< windows

    long tStart = 0;
    long tEnd   = 0;

    LARGE_INTEGER nFreq;
    LARGE_INTEGER nStartTime;
    LARGE_INTEGER nEndTime;
    double time = 0.;

    QueryPerformanceFrequency(&nFreq);
    QFile file( "D:/DriverConfig.xml" );
    QDomDocument doc;

    for( int i = 0; i < TEST_TIMES; ++i )
    {
        doc.clear();

        //< step1 open file
        if( !file.open(QIODevice::ReadOnly) )
        {
            cout << "failed to open file!" << endl;
            continue;
        }
        Sleep( 100 );
        QueryPerformanceCounter(&nStartTime); 

        //< step2 set content
        if( !doc.setContent(&file) )
        {
            cout << "Failed to read xml file!" << endl;
        }
        QueryPerformanceCounter(&nEndTime);
        time = (double)(nEndTime.QuadPart-nStartTime.QuadPart) / (double)nFreq.QuadPart * 1000.;  //< ms
        cout << " seting content costs " << time << "ms" << endl;

        file.close();
        Sleep( 100 );
    }

#else //< LINUX

    timeval starttime, endtime;
    QFile file( "/home/liuyc/DriverConfig.xml" );
    QDomDocument doc;
    double timeuse = 0.;
    double timeAverage = 0.;

    for( int i = 0; i < TEST_TIMES; ++i )
    {
        doc.clear();

        //< step1 open file
        if( !file.open(QIODevice::ReadOnly) )
        {
            cout << "failed to open file!" << endl;
            continue;
        }
        sleep( 1 );  //< delay for 1s
        gettimeofday( &starttime, 0 );

        //< step2 set content
        if( !doc.setContent(&file) )
        {
            cout << "Failed to read xml file!" << endl;
            continue;
        }
        gettimeofday( &endtime, 0 );
        timeuse = 1000000. * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
        timeuse *= 0.001 ;
        timeAverage += timeuse;
        cout << " reading files costs : " << timeuse << "ms" << endl;

        file.close();
        sleep( 1 );  //< delay for 1s
    }

    timeAverage /= TEST_TIMES;
    cout << " The End *****************\n    average costs = " << timeAverage << "ms" << endl; 

#endif

    return a.exec();
}

501-1640ms

当时我的反应是 WTF？？ 为什么同一个函数读同一个文件十次会有这么大的差异所以我才会在文件打开和关闭时分别都加了延时，希望避免文件开关的过程对这个函数产生的影响，结果依然没有解决这个问题，这个问题希望有大神帮我解答一下！

那下面我们来看linux下的运行结果： 

TinyXml-2
下面我们来看利用tinyxml2实现读取的源代码：
#include <iostream>
#include "tinyxml2.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
using namespace tinyxml2;
using std::cout;
using std::endl;

#define TEST_TIMES  10

int main()
{
#ifndef _WIN32  //< linux ------------------------------------------------

    tinyxml2::XMLDocument doc;
    timeval starttime, endtime;
    double timeuse = 0.;
    double timeAverage = 0.;
    for( int i = 0; i < TEST_TIMES; ++i )
    {
        gettimeofday( &starttime, 0 );
        if( XML_SUCCESS != doc.LoadFile( "/home/liuyc/DriverConfig.xml" ) )
        {
            cout << "failed in load xml file! _ " << i << endl;
            continue;
        }
        gettimeofday( &endtime, 0 );

        timeuse = 1000000. * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
        timeuse *= 0.001 ;
        cout << " reading files costs : " << timeuse << "ms" << endl;
        timeAverage += timeuse;
    }
    timeAverage /= TEST_TIMES;
    cout << " \n** The end *******************\n    the average costs = " << timeAverage << "ms" << endl;

#else  //< windows ---------------------------------------------------

    LARGE_INTEGER nFreq;
    LARGE_INTEGER nStartTime;
    LARGE_INTEGER nEndTime;
    double time = 0.;

    QueryPerformanceFrequency(&nFreq);
    tinyxml2::XMLDocument doc;
    for( int i = 0; i < TEST_TIMES; ++i )
    {
        QueryPerformanceCounter(&nStartTime); 
        if( XML_SUCCESS != doc.LoadFile( "D:/DriverConfig.xml" ) )
        {
            cout << "failed in load xml file! _ " << i << endl;
            continue;
        }
        QueryPerformanceCounter(&nEndTime);
        time = (double)(nEndTime.QuadPart-nStartTime.QuadPart) / (double)nFreq.QuadPart * 1000.;  //< ms
        cout << " reading files costs : " << time << "ms" << endl;
    }
    cout << endl;
    system("pause");

#endif  //< end of windows ---------------------------------------------------
    return 0;
}

51-58ms

RapidXml

注：RapidXml版本： 1.13 
在RapidXml Manual的介绍中可以看到它和TinyXml以及其他的一些xml解析库做了对比（这里面tinyXml是最慢的），原文中介绍这是目前Xml解析最快的

As a rule of thumb, parsing speed is about 50-100x faster than Xerces DOM, 30-60x faster than TinyXml, 3-12x faster than pugxml, and about 5% - 30% faster than pugixml, the fastest XML parser I know of.
所以这里我也想要试试看RapidXml在内容解析时的效率表现，下面是源代码：
#include <iostream>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"
#ifdef _WIN32
# include <Windows.h>
#else
# include <sys/time.h>
#endif

using namespace rapidxml;
using std::cout;
using std::endl;

#define TEST_TIMES  10

int main()
{
#ifdef _WIN32  //< windows

    LARGE_INTEGER nFreq;
    LARGE_INTEGER nStartTime;
    LARGE_INTEGER nEndTime;
    double time = 0.;
    QueryPerformanceFrequency(&nFreq);

    //< parse xml
    for( int i = 0 ; i < TEST_TIMES; ++i )
    {
        rapidxml::file<> filename( "D:/DriverConfig.xml" );
        xml_document<> doc;
        QueryPerformanceCounter(&nStartTime); 

        doc.parse<0>( filename.data() );

        QueryPerformanceCounter(&nEndTime);
        time = (double)(nEndTime.QuadPart-nStartTime.QuadPart) / (double)nFreq.QuadPart * 1000.;  //< ms
        cout << " reading files costs : " << time << "ms" << endl;
        doc.clear();
    }

    system("pause");

#else

    timeval starttime, endtime;
    double timeuse = 0.;
    double timeAverage = 0.;

    //< parse xml
    for( int i = 0 ; i < TEST_TIMES; ++i )
    {
        rapidxml::file<> filename( "/home/liuyc/DriverConfig.xml" );
        xml_document<> doc;
        gettimeofday( &starttime, 0 );

        doc.parse<0>( filename.data() );

        gettimeofday( &endtime, 0 );

        timeuse = 1000000. * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
        timeuse *= 0.001 ;
        cout << " reading files costs : " << timeuse << "ms" << endl;
        doc.clear();

        timeAverage += timeuse;
    }
    timeAverage /= TEST_TIMES;
    cout << " \n** The end *******************\n    the average costs = " << timeAverage << "ms" << endl;

#endif

    return 0;
}
效率确实为 TinyXml2 的2.x倍，但是并没有像 rapidXml 的说明手册里说的有30~60倍的效率差异（手册中是和TinyXml比较的，而不是TinyXml2），这里我不清楚到底是TinyXml2相对TinyXml有了明显的效率提升还是我在RapidXml的使用上有问题，后面需要再仔细调查RapidXml的接口使用方法。 
在我自己初步的使用来看，我觉得 RapidXml 的接口并有 Qt 和 TinyXml2 那么简单易用，所以在文件大小不大或对效率要求不是很极限的情况下，使用 TinyXml2 可能会获得开发效率和运行效率的双赢。

再来看一下windows下的运行结果： 


PugiXml

经博友提醒，现添加pugixml的测试，为节省篇幅只贴出 linux 下的测试代码（代码与 tinyxml 非常相似），可以看到pugixml与tinyxml的调用方式很相似！
#include <iostream>
#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include <sys/time.h>
using namespace std;

#define TEST_TIMES 10

int main( void )
{
    pugi::xml_document doc;
    timeval starttime, endtime;
    double timeuse = 0.;
    double timeAverage = 0.;
    for( int i = 0; i < TEST_TIMES; ++i )
    {
        gettimeofday( &starttime, 0 );
        if( !doc.load_file( "/home/liuyc/DriverConfig.xml" ) )
        {
            cout << "failed in load xml file! _ " << i << endl;
            continue;
        }
        gettimeofday( &endtime, 0 );

        timeuse = 1000000. * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
        timeuse *= 0.001 ;
        cout << " reading files costs : " << timeuse << "ms" << endl;
        timeAverage += timeuse;
    }
    timeAverage /= TEST_TIMES;
    cout << " \n** The end *******************\n    the average costs = " << timeAverage << "ms" << endl;
    return 0;
}

总结

统计的时间如下（LINUX）：
解析器	消耗时间(ms)	效率倍数(相对Qt)
Qt-QDomDocument	25.85	1
TinyXml2	6.64	3.89
RapidXml	2.71	9.54
pugixml	1.57	16.47
工作以来基本上都是在Qt下开发，深切体会到Qt的接口封装很完善易用，但不可避免的牺牲了一些效率（虽然没想到效率降低了这么多），相对的，tinyxml2和pugixml在接口上都与Qt非常相似，Pugixml的效率更是提升明显，所以目前xml的解析还是推荐Pugixml。后面我会继续仔细研究各类xml解析器的使用方法，再与大家分享！
