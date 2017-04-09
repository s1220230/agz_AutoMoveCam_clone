#include "Xbee_com.h"
#include "Img_proc.h"
#include "Control.h"
#include <conio.h>
#include <fstream>
#include <time.h>

#define GRAVITY 1      //画像中の領域 : 0  注目領域 : 1
#define CAM_ID 1	   //カメラID
const LPCSTR com = "COM5";		//COMポート名
std::vector<cv::Point2f> Pos;	//水田の四隅の点
std::vector<cv::Point2f> Pos2;

cv::Point2i target, P0[5] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, P1 = { 0, 0 };
cv::Point2i pre_point; // @comment Point構造体<int型>
int action;			   //ロボットの動作変数 1:前進 2:右旋回 4:左

SOM s = SOM();
SOM s2 = SOM();

//cv::Mat sample_img = cv::imread("test2.JPG", 1);            //テスト画像用（SOM）
//cv::Mat sample_img = cv::imread("rice.jpg", 1);            //テスト画像用（SOM）
//if (!sample_img.data)exit(0);								//テスト画像用（SOM）
//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //テスト画像用（SOM）


//データ出力用csvファイル名作成関数　
std::string setFilename(std::string str){
	time_t now = time(NULL);
	struct tm * pnow = localtime(&now);
	char time[32];
	std::string c = ".csv";
	std::string data = "./data/";

	//@comment sprintfを使ってint型をstringに変換
	sprintf(time, "%d_%d_%d_%d_%d", pnow->tm_year + 1900, pnow->tm_mon + 1,
		pnow->tm_mday, pnow->tm_hour, pnow->tm_min);

	return data + str + time + c; //@comment ファイル名
}

//マウスクリック取得座標位置に円をプロット
void drawPoint(cv::Mat* img, cv::Point2i point){
	cv::circle(*img, point, 8, cv::Scalar(0, 255, 0), -1, CV_AA);
	cv::imshow("getCoordinates", *img);
}

//水田領域の座標取得用関数
void getCoordinates(int event, int x, int y, int flags, void* param)
{
	cv::Mat* image = static_cast<cv::Mat*>(param);
	static int count = 0;
	switch (event) {
	case CV_EVENT_LBUTTONDOWN://@comment 左クリックが押された時
		Pos.push_back(cv::Point2f(x, y));
		drawPoint(image, cv::Point2i(x, y));
		std::cout << ++count << " : " << "x : " << x << ", y : " << y << std::endl;
		if (count > 3){
			cv::imshow("getCoordinates", *image);
			cv::waitKey(200);
			cv::destroyAllWindows();
			break;
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		//4点以上クリックされない場合画像の内側の4点を指定
		if (count < 4){

			std::cout << std::endl;
			std::cout << "4点以上指定してください" << std::endl<<std::endl;
			system("pause");
			exit(0);
		}
		cv::destroyAllWindows();
		break;
	}
}


//画像を取得し,水田領域を設定
void setUp(LPCSTR com, HANDLE &hdl, Img_Proc &imp){
	int width=10, height=10;
	cv::Mat field;
	cv::UMat src_frame, dst_img;


	//std::cout << "水田の大きさを入力してください(m)単位" << std::endl;
	//std::cout << "横 : ";    std::cin >> width;
	//std::cout << "縦 : ";    std::cin >> height;
	//std::cout << std::endl;

	width *= 100;
	height *= 100;

	//if (width < 300 || height < 300) //@comment 3x3(m)以上の領域を指定
	//{
	//	std::cout << "※ 縦、横それぞれ３ｍ以上を指定してください" << std::endl;
	//	std::cout << std::endl;
	//	system("PAUSE");
	//	exit(0);
	//}

	imp.setField(width, height);
	nm30_init();
	nm30_set_panorama_mode(1, 11); //@comment 魚眼補正

	//@comment 始めの１０フレームは読み飛ばす
	for (int i = 0; i < 10; i++) {
		imp.getFrame().copyTo(src_frame);//@comment 1フレーム取得
	}
	//if (!sample_img.data)exit(0);								//テスト画像用（SOM）
	//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //テスト画像用（SOM）
	//sample_img.copyTo(src_frame); //テスト画像用（SOM）
	std::cout << "水田の領域を左下から時計回りになるように４点クリックしてください" << std::endl;

	//------------------座標取得-----------------------------------------------
	//画像中からマウスで4 ~ 10点を取得その後右クリックすると変換処理が開始する
	cv::namedWindow("getCoordinates");
	cv::imshow("getCoordinates", src_frame);
	src_frame.copyTo(field);
	cv::setMouseCallback("getCoordinates", getCoordinates, (void *)&field);

	cv::waitKey(0);
	cv::destroyAllWindows();

	//------------------透視変換-----------------------------------------------

	imp.Perspective(src_frame, dst_img, Pos);

	Pos2.push_back(cv::Point(30, dst_img.rows - 30));
	Pos2.push_back(cv::Point(30, 30));
	Pos2.push_back(cv::Point(dst_img.cols - 30, 30));
	Pos2.push_back(cv::Point(dst_img.cols - 30, dst_img.rows - 30));

	cv::Point pt2[10]; //任意の4点を配列に入れる
	for (int i = 0; i < Pos2.size(); i++){
		pt2[i] = Pos2[i];
	}

	s2.set_size(width, height);
	s2.set_pos(Pos2);
	s2.set_img(dst_img);
	cv::Mat_<cv::Vec3b> ss2(dst_img.size());
	for (int j = 0; j < dst_img.rows; j++){
		for (int i = 0; i < dst_img.cols; i++){
			ss2(j, i) = cv::Vec3b(255, 255, 255);
		}
	}
	cv::fillConvexPoly(ss2, pt2, Pos.size(), cv::Scalar(200, 200, 200));//多角形を描画
	s2.Init2(ss2);
	cv::destroyAllWindows();
}

//制御ループ
void Moving(HANDLE &arduino, Xbee_com &xbee, Img_Proc &imp){
	cv::Mat element = cv::Mat::ones(3, 3, CV_8UC1); //2値画像膨張用行列
	cv::Mat heatmap_img(cv::Size(500, 500), CV_8UC3, cv::Scalar(255, 255, 255));
	int frameNum = 0;								//フレーム数保持変数
	cv::UMat src, dst, colorExtra, pImg, binari_2, copyImg, copyImg2;
	cv::Point2f sz = imp.getField();
	Control control(sz.x, sz.y);
	control.set_target(s2);
	char command = 's';
	int ypos;
	int ydef = 0;	//補正なし重心座標値
	int num = 0;	// ターゲットの訪問回数更新
	std::ofstream ofs(setFilename("Coordinate")); // @comment　ロボット座標用csv

	//ロボット座標csv
	ofs << imp.getField().x << ", " << imp.getField().y << std::endl;
	ofs << "x軸, y軸（補正なし）, ypos（補正あり）" << std::endl;

	////////////////////////////////////////////////////////////////
	//ロボット色抽出テスト用
	//int h_value = 180,h_value2 = 0,s_value = 70,v_value = 70;
	//cv::namedWindow("colorExt", 1);
	//cv::createTrackbar("Hmax", "colorExt", &h_value, 180);
	//cv::createTrackbar("Hmin", "colorExt", &h_value2, 180);
	//cv::createTrackbar("S", "colorExt", &s_value, 255);
	//cv::createTrackbar("V", "colorExt", &v_value, 255);
	///////////////////////////////////////////////////////////////

	std::cout << "a: 自動掃引" << std::endl;
	std::cout << "s: 停止" << std::endl;
	std::cout << "q もしくはウィンドウ右上の×ボタン : 終了" << std::endl;

	while (1){
		imp.getFrame().copyTo(src);
		//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //テスト画像用（SOM）
		//sample_img.copyTo(src); //テスト画像用（SOM）
		if (frameNum % 1 == 0){
			if (_kbhit()){
				command = _getch();
				if (command == 's') {
					xbee.sentManualCommand(byte(0x00), arduino);
					std::cout << "停止" << std::endl << std::endl;
				}
				if (command == 'q') {
					xbee.sentManualCommand(byte(0x01), arduino);
					std::cout << "終了 " << std::endl << std::endl;
					cv::destroyAllWindows();
					ofs.close(); //@comment ファイルストリームの解放
					break;
				}
				if (command == 'a') {
					xbee.sentManualCommand(byte(0x01), arduino);
					std::cout << "自動掃引" << std::endl << std::endl;
				}
			}
			//@comment 画像をリサイズ(大きすぎるとディスプレイに入りらないため)
			src.copyTo(copyImg);
			cv::warpPerspective(src, dst, imp.getPersMat(), cv::Size(src.cols, src.rows), CV_INTER_LINEAR);
			dst.copyTo(copyImg2);

			//@comment hsvを利用して赤色を抽出
			//入力画像、出力画像、変換、h最小値、h最大値、s最小値、s最大値、v最小値、v最大値 h:(0-180)実際の1/2
			//imp.colorExtraction(dst, colorExtra, CV_BGR2HSV, h_value, h_value2, s_value, 255, v_value, 255);
			imp.colorExtraction(dst, colorExtra, CV_BGR2HSV, 160, 10, 70, 255, 70, 255);
			colorExtra.copyTo(pImg);
			cv::cvtColor(colorExtra, colorExtra, CV_BGR2GRAY);//@comment グレースケールに変換

			//----------------------二値化-----------------------------------------------
			cv::threshold(colorExtra, binari_2, 0, 255, CV_THRESH_BINARY);
			cv::dilate(binari_2, binari_2, element, cv::Point(-1, -1), 3); //膨張処理3回 最後の引数で回数を設定

			//---------------------面積計算,重心取得-----------------------------------------------
			//取得した領域の中で一番面積の大きいものを対象としてその対象の重心を求める。
			cv::Point2i point = imp.serchMaxArea(binari_2, pImg);
			if (!GRAVITY)
			{
				point = imp.calculate_center(binari_2);//@comment momentで白色部分の重心を求める
				std::cout << "posion: " << point.x << " " << point.y << std::endl;//@comment 重心点の表示
			}

			if (point.x != 0) {
				ypos = sz.y - (point.y + 6 * ((1000 / point.y) + 1));
				ydef = sz.y - point.y;//@comment 補正なしｙ重心
				//std::cout << point.x << " " << ypos << std::endl; //@comment 変換画像中でのロボットの座標(重心)
				ofs << point.x << ", " << ydef << ", " << ypos << std::endl; //@comment 変換
			}

			//---------------------ロボットの動作取得------------------------------------
			//if (frame % 2 == 0){
			P1 = cv::Point2f(point.x, sz.y - ydef);
			if (P1.x != 0 && P1.y != 0) {
				// ターゲットの更新
				if (control.is_updateTarget()){
					//std::cout << "target number : " << control.get_target() << std::endl;
				}
				// 現在のロボットの位置情報の更新
				control.set_point(P1);

				// ロボットの動作決定
				action = control.robot_action(P0[4]);

				// ターゲットの訪問回数更新
				//num = control.target_count();

				control.heatmap(control.area_count(), &heatmap_img, &imp.makeColorbar());
				// 内外判定
				control.is_out();
				for (int i = 1; i < 5; i++){
					P0[i] = P0[i - 1];
				}
			}

			else{
				action = 0;
			}
			P0[0] = P1;
			//} //ロボットの動作取得

			if (command == 'a'){
				xbee.sentAigamoCommand(action, arduino);
			}
			////std::cout << "cmd " << int(command) << std::endl;
			cv::UMat test, test2;
			src.copyTo(test);
			dst.copyTo(test2);

			cv::warpPerspective(test2, test, imp.getInvPerse(), cv::Size(test.cols, test.rows), CV_INTER_LINEAR);


			//-------------------重心点のプロット----------------------------------------- 
			if (!point.y == 0) { //@comment point.y == 0の場合はexceptionが起こる( 0除算 )
				
				circle(copyImg, imp.calcHomoPoint(cv::Point2f(point)), 8, cv::Scalar(0, 0, 255), -1, CV_AA);
				circle(dst, cv::Point(point.x, point.y), 8, cv::Scalar(0, 0, 255), -1, CV_AA);
			}

			//------------------ターゲットのプロット--------------------------------------

			control.plot_target(dst, P0[4]);
			control.plot_transform_target(copyImg, P0[4], imp.getInvPerse());

			//------------------マス, 直進領域のプロット--------------------------------------

			s2.showSOM3(dst,control.get_nowTargetArea());
			s2.showSOM2(copyImg, imp.getInvPerse(),control.get_nowTargetArea());

			//---------------------表示部分----------------------------------------------

			cv::imshow("dst_image", dst);//@comment 出力画像
			cv::imshow("copyImg", copyImg);
			
			cv::waitKey(1);

			//@comment "q"を押したらプログラム終了
			//if (src.empty() || cv::waitKey(50) == 113)
			//{
			//	cv::destroyAllWindows();
			//	ofs.close(); //@comment ファイルストリームの解放
			//	break;
			//}
		}
		frameNum++;
	} 
	ofs.close(); //@comment ファイルストリームの解放
}

//ロボット自動制御
void AutoMove(){
	HANDLE hdl;							//COMポート通信ハンドル
	Img_Proc imp = Img_Proc(CAM_ID);    //画像処理用クラス
	Xbee_com xbee = Xbee_com(com, hdl); 	//xbee通信の初期化
	setUp(com, hdl, imp);				//初期セットアップ
	Moving(hdl, xbee, imp);				//自動制御ループ
}

int main(int argc, char *argv[]){
	AutoMove();
	return 0;
}