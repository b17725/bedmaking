//
//  main.cpp
//  skeleton_test
//
//  Created by students on 2018/07/11.
//  Copyright © 2018年 students. All rights reserved.
//
#include <fstream>
#include <iostream>
#include <string>

#include <NiTE.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <cstdlib>
#include <stdio.h>

using namespace std;

int waistangle_flag = 0;

class NiteApp
{
public:
    
    // 初期化
    void initialize()
    {
        // UserTracker を作成する
        userTracker.create();
    }
    
    // フレーム更新処理
    void update()
    {
        // ユーザーフレームを取得する
        nite::UserTrackerFrameRef userFrame;
        userTracker.readFrame( &userFrame );
        
        // ユーザー位置を描画する
        depthImage = showUser( userFrame );
        
        // 検出したユーザーを取得する
        const nite::Array<nite::UserData>& users = userFrame.getUsers();
        for ( int i = 0; i < users.getSize(); ++i ) {
            const nite::UserData& user = users[i];
            
            // 新しく検出したユーザーの場合は、スケルトントラッキングを開始する
            if ( user.isNew() ) {
                userTracker.startSkeletonTracking( user.getId() );
            }
            // すでに検出したユーザーで、消失していない場合は、スケルトンの位置を表示する
            else if ( !user.isLost() ) {
                showSkeleton( depthImage, userTracker, user );
            }
        }
        
        //  描画
        cv::imshow( "Skeleton", depthImage );
    }
    
private:
    
    // ユーザーの検出
    cv::Mat showUser( nite::UserTrackerFrameRef& userFrame )
    {
        // ユーザーにつける色
        static const cv::Scalar colors[] = {
            cv::Scalar( 0, 0, 1 ),
            cv::Scalar( 1, 0, 0 ),
            cv::Scalar( 0, 1, 0 ),
            cv::Scalar( 1, 1, 0 ),
            cv::Scalar( 1, 0, 1 ),
            cv::Scalar( 0, 1, 1 ),
            cv::Scalar( 0.5, 0, 0 ),
            cv::Scalar( 0, 0.5, 0 ),
            cv::Scalar( 0, 0, 0.5 ),
            cv::Scalar( 0.5, 0.5, 0 ),
        };
        
        cv::Mat depthImage;
        
        
        // Depth フレームを取得する
        openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
        if ( depthFrame.isValid() ) {
            depthImage = cv::Mat( depthFrame.getHeight(),
                                 depthFrame.getWidth(),
                                 CV_8UC4 );
            
            // Depth データおよびユーザーインデックスを取得する
            openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
            const nite::UserId* pLabels = userFrame.getUserMap().getPixels();
            
            // 1ピクセルずつ調べる
            for ( int i = 0; i < (depthFrame.getDataSize() / sizeof(openni::DepthPixel)); ++i ) {
                // カラー画像インデックスを生成
                int index = i * 4;
                
                // 距離データを画像化する
                uchar* data = &depthImage.data[index];
                if ( pLabels[i] != 0 ) {
                    // 人を検出したピクセルにはユーザー番号で色を付ける
                    data[0] *= colors[pLabels[i]][0];
                    data[1] *= colors[pLabels[i]][1];
                    data[2] *= colors[pLabels[i]][2];
                }
                else {
                    // 人を検出しなかったピクセルは Depth データを書きこむ
                    // 0-255のグレーデータを作成する
                    // distance : 10000 = gray : 255
                    int gray = ~((depth[i] * 255) / 10000);
                    data[0] = gray;
                    data[1] = gray;
                    data[2] = gray;
                }
            }
        }
        
        return depthImage;
    }
    
    // スケルトンの検出
    void showSkeleton( cv::Mat& depthImage, nite::UserTracker& userTracker, const nite::UserData& user )
    {
        // スケルトンを取得し、追跡状態を確認する
        const nite::Skeleton& skeelton = user.getSkeleton();
        if ( skeelton.getState() != nite::SKELETON_TRACKED ) {
            return;
        }
        
        // すべての関節を描画する
        for ( int j = 0; j <= 14; ++j ) {
            //-----フラグ初期化
            j_flag[j] = 0;
            JOINT[j][0]=0; JOINT[j][1]=0; JOINT[j][2]=0;
            
            // 関節情報を取得し、信頼度の数値が一定以上の場所のみ表示する
            const nite::SkeletonJoint& joint = skeelton.getJoint( (nite::JointType)j );
            if ( joint.getPositionConfidence() < 0.7f ) {
                continue;
            }
            
            //-----各関節で信用度を有無を記録
            j_flag[j] = 1;
            
            // 3次元の座標を2次元の座標に変換する
            const nite::Point3f& position = joint.getPosition();
            float x = 0, y = 0;
            userTracker.convertJointCoordinatesToDepth(
                                                       position.x, position.y, position.z, &x, &y );
//            if (j==1) {//setprecision(7)  有効数字桁
//                cout << setprecision(7) <<"x1="
//                    << position.x <<",y1= "
//                    << position.y <<",z1= "
//                    << position.z << endl;
//            }
            
            //-----信用度が取れている関節の座標保存
            JOINT[j][0] = position.x;
            JOINT[j][1] = position.y;
            JOINT[j][2] = position.z;
            
            switch (j) {
                case 6://左手
                    cv::circle( depthImage, cvPoint( (int)x, (int)y ),
                               5, cv::Scalar( 255, 0, 255 ), -1 );//bgr  紫
                    break;
                case 7://右手
                    cv::circle( depthImage, cvPoint( (int)x, (int)y ),
                               5, cv::Scalar( 0, 255, 255 ), -1 );//bgr 黄
                    break;
                    
                default:
                    // 円を表示する
                    cv::circle( depthImage, cvPoint( (int)x, (int)y ),
                               5, cv::Scalar( 0, 0, 255 ), -1 );
                    break;
            }
            
        }
        
        //-----腰(仙骨部分)の算出開始
        if (j_flag[9] == 1 && j_flag[10] == 1) {
            float sacrum_x = 0, sacrum_y = 0;
            double sacrum_posX, sacrum_posY, sacrum_posZ;
            
            //-----右腰と左腰の中間にある腰（仙骨部分）の算出
            sacrum_posX = (JOINT[9][0] + JOINT[10][0]) / 2.0;
            sacrum_posY = (JOINT[9][1] + JOINT[10][1]) / 2.0;
            sacrum_posZ = (JOINT[9][2] + JOINT[10][2]) / 2.0;
            
//            cout << setprecision(7) <<"x2="
//            << sacrum_posX <<",y2= "
//            << sacrum_posY <<",z2= "
//            << sacrum_posZ << endl;
            
            
            userTracker.convertJointCoordinatesToDepth(
                        sacrum_posX, sacrum_posY, sacrum_posZ, &sacrum_x, &sacrum_y );
            //-----腰（仙骨部分）を表示する
            cv::circle( depthImage, cvPoint( (int)sacrum_x, (int)sacrum_y ),
                       5, cv::Scalar( 0, 255, 0 ), -1 );
            
            //-----右腰左腰につづき、首の信用度もあるなら腰角度の計算
            if (j_flag[1] == 1) {
                double bunsi, bunbo;
                
                waist_vecA[0] =sacrum_posX - JOINT[1][0];//vecA.x
                waist_vecA[1] =sacrum_posY - JOINT[1][1];//vecA.y
                waist_vecA[2] =sacrum_posZ - JOINT[1][2];//vecA.z
                waist_vecB[0] =sacrum_posX - JOINT[1][0];//vecB.x
                waist_vecB[1] =0;//vecB.y
                waist_vecB[2] =sacrum_posZ - JOINT[1][2];//vecB.z
//                waist_vecA[0] =sacrum_posX - JOINT[1][0];//vecA.x
//                waist_vecA[1] =sacrum_posY - JOINT[1][1];//vecA.y
//                waist_vecA[2] =sacrum_posZ - JOINT[1][2];//vecA.z
//                waist_vecB[0] =sacrum_posX;//vecB.x
//                waist_vecB[1] =JOINT[1][1];//vecB.y
//                waist_vecB[2] =sacrum_posZ;//vecB.z
                
                bunsi = ((waist_vecA[0]*waist_vecB[0]) +
                         (waist_vecA[1]*waist_vecB[1]) +
                         (waist_vecA[2]*waist_vecB[2]));
                bunbo = sqrt(waist_vecA[0]*waist_vecA[0]+waist_vecA[1]*waist_vecA[1]+waist_vecA[2]*waist_vecA[2]) *
                        sqrt(waist_vecB[0]*waist_vecB[0]+waist_vecB[1]*waist_vecB[1]+waist_vecB[2]*waist_vecB[2]);
                
                //腰角度 cosθ = Waist_cossheta
                Waist_cossheta = bunsi / bunbo;
                //cosθ = Waist_cossheta  ->  θ = WaistAngle
                WaistAngle = 90.0 - (acos(Waist_cossheta) * unit_r);
                
                //警告音再生
                if (musicflag == 0) {
                    if (WaistAngle > 30.0) {
                        
                        FIVEsec_timer++;// 傾き続けてる秒数
                        
                        if (FIVEsec_timer >= 30) {//3秒以上傾きつづけてる
                            // filename.mp3を再生
                            system("afplay /Users/students/Desktop/UserViewer0625/skeleton_test/music/a.wav&");
                            cout << fixed;
                            cout << setprecision(4) << "θ= " << WaistAngle << endl;//腰角度の印字、小数点第6位まで
                            musicflag++;
                        }
                    }else{
                        FIVEsec_timer = 0;
                    }
                }else{
                    musicflag++;
                    if (musicflag > 30) {
                        musicflag = 0;
                        FIVEsec_timer = 0;
                    }
                }
//                //旧。
//                if (WaistAngle > 30.0 && musicflag == 0) {
//
//                    FIVEsec_timer++;
//
//                    if (FIVEsec_timer >= 150) {//5秒以上傾きつづけてる
//                        // filename.mp3を再生
//                        system("afplay /Users/students/Desktop/UserViewer0625/skeleton_test/music/a.wav&");
//                        cout << fixed;
//                        cout << setprecision(4) << "θ= " << WaistAngle << endl;//腰角度の印字、小数点第6位まで
//                        musicflag++;
//                    }
//                }else{
//                    musicflag++;
//                    if (musicflag > 30) {
//                        musicflag = 0;
//                        FIVEsec_timer = 0;
//                    }
//                }
                
                //印字
                if (waistangle_flag == 1) {
//                    cout << setprecision(7) <<"x1="
//                    << waist_vecA[0] <<",y1= "
//                    << waist_vecA[1] <<",z1= "
//                    << waist_vecA[2] << endl;
//
//                    cout << setprecision(7) <<"x2="
//                    << waist_vecB[0] <<",y2= "
//                    << waist_vecB[1] <<",z2= "
//                    << waist_vecB[2] << endl;
//
//                    //fixedはsetprecisionを小数点以下から開始 -> std::setprecision(2) << 3.1415; // "3.14"
//                    cout << fixed;
//                    cout << setprecision(4) << "cosθ= " << Waist_cossheta << endl;//cosθの印字、小数点第6位まで
                    cout << setprecision(4) << "θ= " << WaistAngle << endl;//腰角度の印字、小数点第6位まで
                    
                    waistangle_flag = 0;
                }
                
//                //fixedはsetprecisionを小数点以下から開始 -> std::setprecision(2) << 3.1415; // "3.14"
//                cout << fixed;
//                cout << setprecision(4) << WaistAngle << endl;//腰角度の印字、小数点第6位まで
                
            }
        }
        
    }
    /*
     1:頭 ,2:首 ,3:左肩 ,4:右肩 ,5:左肘 ,
     6:右肘 ,7:左手 ,8:右手 ,9:胴 ,10:左腰 ,
     11:右腰 ,12:左ひざ ,13:右ひざ ,14:左足 ,15:右足 ,
     system("afplay /Users/students/Desktop/UserViewer0625/skeleton_test/music/a.wav"); // filename.wavを再生
     */
    
private:
    
    nite::UserTracker userTracker;  // ユーザー検出
    
    cv::Mat depthImage;             // 可視化した Depth データ
    double JOINT[15][3] = {0.0};//関節情報の保存用
    int j_flag[15]={0};//各関節で信用度を有無を記録
    double waist_vecA[3];
    double waist_vecB[3];
    double Waist_cossheta = 0.0;//腰角度
    double WaistAngle = 0.0;//腰角度
    //double pi     = 2.0 * asin(1.0);/* πの値  M_PI */
    //double unit   = M_PI / 180.0;/* 度 → ラジアン */
    double unit_r = 180.0 / M_PI;/* ラジアン → 度 */
    int musicflag=0;
    int FIVEsec_timer=0;
    
};

int main(int argc, const char * argv[])
{
    cout << "Hello, World!\n" << endl;
    std::ifstream ifs("/Users/students/Desktop/UserViewer0625/skeleton_test/testfile.txt");
    int buf_size = 31;
    char str[buf_size];
    if (ifs.fail()) {
        std::cerr << "Failed to open file." << std::endl;
        //return -1;
    }
    while (ifs.getline(str, buf_size)) {
        std::cout << "#" << str << std::endl;
    }
    return 0;
//    try {
//        // NiTE を初期化する
//        nite::NiTE::initialize();
//
//        // アプリケーションの初期化
//        NiteApp app;
//        app.initialize();
//
//        // メインループ
//        while ( 1 ) {
//            app.update();
//
//            int key = cv::waitKey( 10 );
//            if ( key == 'q' ) {
//                std::cout << "System finish.\n";
//                break;
//            }
//            if ( key == 's' ) {
//                waistangle_flag = 1;
//            }
//
//        }
//    }
//    catch ( std::exception& ) {
//        std::cout << openni::OpenNI::getExtendedError() << std::endl;
//    }
    
    return 0;
}
/*FILE* fp; //ファイルの場所を保持する入れ物　FILE*は型の名前
 //下は出力するテキストファイルのパスを指定する文
 char filename[] = "output¥¥output.txt"; //Windowsの場合　パス中の¥は重ねて¥¥とする
 char filename[] = "output/output.txt"; //Macの場合
 fp = fopen(filename, "w"); //"w"は書き込み可能な状態でファイルを開くことを指定
 for(int i=0; i<10;i++){
 fprintf(fp, "%dの2乗は%dです\n",i , i*i); //ファイルに書き込み
 }
 fclose(fp); //ファイルを閉じる*/

