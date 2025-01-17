
#include "MainWindow.h"
#include <QtWidgets/QApplication>

#include <QDebug>
#include <SDL.h>


#undef main


// Fault tolerant heap shim applied to current process.
// This is due to previous crash.
/*
���������
1����ע���regedit
2���ҵ�HKEY_LOCAL_MACHINE\Software\Microsoft\Windows  NT\CurrentVersion\AppCompatFlags\Layers
3��ɾ��exe��Ӧ��key
*/

int main(int argc, char *argv[])
{
    //    RawVideoFile in = {
    //        "F:/res/in.yuv",
    //        512, 512, AV_PIX_FMT_YUV420P
    //    };
    //    RawVideoFile out = {
    //        "F:/res/dragon_ball.rgb",
    //        512, 512, AV_PIX_FMT_RGB24
    //    };
    //    FFmpegs::convertRawVideo(in, out);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/*
640*480��yuv420p

---- 640��Y -----
YY............YY |
YY............YY |
YY............YY |
YY............YY
................ 480��
YY............YY
YY............YY |
YY............YY |
YY............YY |
YY............YY |

---- 320��U -----
UU............UU |
UU............UU |
UU............UU |
UU............UU
................ 240��
UU............UU
UU............UU |
UU............UU |
UU............UU |
UU............UU |

---- 320��V -----
VV............VV |
VV............VV |
VV............VV |
VV............VV
................ 240��
VV............VV
VV............VV |
VV............VV |
VV............VV |
VV............VV |

600*600��rgb24

-------  600��RGB ------
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB 600��
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |

6 * 4��yuv420p

YYYYYY
YYYYYY
YYYYYY
YYYYYY

UUU
UUU

VVV
VVV
*/
