//
//  main.cpp
//  faceACapture
//
//  Created by sawano on 2018/04/07.
//  Copyright © 2018年 AIT. All rights reserved.
//


#include <iostream>
#include <stdio.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/highgui/highgui.hpp"
//OpenCV用のヘッダファイル
#include <opencv2/opencv.hpp>


//画像サイズ
#define WIDTH (640)
#define HEIGHT (480)
#define WINDOW_NAME "input"
#define SAVE_FILE_NAME "input.jpg"
using namespace cv;
using namespace std;


int main(int argc, const char * argv[])
{
    cv::VideoCapture cap(0);//デバイスのオープン
    
    if(!cap.isOpened()){//カメラデバイスが正常にオープンしたか確認．
        return -1; //読み込みに失敗したときの処理
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH); //幅の設定
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT); //高さの設定
    
    
    //VideoWriter writer("output.avi", CV_FOURCC_DEFAULT, 30, cv::Size(600, 600), true);//デフォ
    //VideoWriter writer("/Users/students/Desktop/test/mov/output_mp4v.mp4", VideoWriter::fourcc('M', 'P', '4', 'V'), 30, cv::Size(640, 480), true);
    VideoWriter writer("/Users/students/Desktop/test/mov/output_h264_1016.mp4", VideoWriter::fourcc('h', '2', '6', '4'), 30, cv::Size(640, 480), true);//できなかった
    
//    for(int i=0; i<max_frame;i++){
//        cap>>img ;
//        writer << img;
//        imshow("Video",img);
//        waitKey(1);
//    }
    int c=0;
    ifstream ifs;
    while(1)//無限ループ
    {
        cv::Mat frame;
        cap >> frame; // get a new frame from camera
        // 水平反転
        cv::flip(frame, frame, 1);
        
        writer << frame;
        cv::imshow(WINDOW_NAME, frame);//画像を表示．
        
        int key = cv::waitKey(1);
        if(key == 'q')//qボタンが押されたとき
        {
            break;//whileループから抜ける．
        }else if(key == 's'){//sが押されたとき
            //フレーム画像を保存する．
            cv::imwrite(SAVE_FILE_NAME, frame);
            fprintf(stderr, "save file.\n");
        }
        /*--- もしファイルが存在する場合（中身の有無にかかわらず）、breakする---*/
        ifs.open("/Users/students/Desktop/test/bin/testfile.txt");
        if (ifs.is_open()) {
            //std::cerr << "open file." << std::endl;
            ifs.close();
            break;
        }
        ifs.close();
        /*-----------------------------------------------------------*/
        
        
        
//        c = getchar();//これだと毎周止まっちゃう
//        printf("%d\n", c);
//        if (c == 'e')            /* 入力された文字が 'e' なら 、「１０」はEOF???*/
//            break;
    }
    
    cap.release();
    writer.release();
    cv::destroyAllWindows(); //ウィンドウの削除
    return 0;
}
