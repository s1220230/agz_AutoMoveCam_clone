#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/features2d/features2d.hpp>
//#include <opencv2/xfeatures2d/nonfree.hpp>
//#include "SOM.h"

class Img_Proc
{
private:
	//掃引領域
	cv::Point2i field;

	//透視変換行列
	cv::Mat perspective_matrix;

	//カメラストリーム
	cv::VideoCapture cap;

	//取得画像
	cv::UMat capImg;

	cv::Mat InvPerse_matrix;

	cv::Mat Homo;
public:
	//カメラの設定
	Img_Proc(int camId);

	//水田領域の設定
	void setField(int w, int h);

	//カラーバーの作成
	cv::Mat makeColorbar();

	//カラーバーのパラメータ調整
	cv::Vec3b calcPseudoColor(double phase);

	//領域の取得
	cv::Point2i getField();

	//透視変換行列の取得
	cv::Mat getPersMat();

	//最大領域の取得
	cv::Point2i serchMaxArea(cv::UMat &src, cv::UMat &plot);

	//中心の算出
	cv::Point2i calculate_center(cv::UMat &);

	//歪み補正(Gopro)
	cv::Mat undist(cv::Mat); //@comment 歪み補正(gopro)

	//色抽出用関数
	void colorExtraction(cv::UMat &src, cv::UMat &dst,
		int code,
		int ch1Lower, int ch1Upper,
		int ch2Lower, int ch2Upper,
		int ch3Lower, int ch3Upper
		);

	//画像取得関数
	cv::UMat getFrame();

	//透視変換関数
	void Perspective(cv::UMat &src, cv::UMat &dst, std::vector<cv::Point2f> &p);

	//領域のプロット
	void plot_field(cv::UMat &src, cv::Point2f sz);

	cv::Mat Img_Proc::getInvPerse();
	//void plot_SOM(cv::UMat &src, SOM &som);

	cv::Point2f calcHomoPoint(cv::Point2f &p);
	cv::Mat calcHomo(cv::UMat &img, cv::UMat &img2);

	void setH(cv::Mat H);

	std::vector<cv::Point2f> getPerseArea(std::vector<cv::Point2f> &Pos);
	//void perseSOM(cv::UMat &img, SOM &som);
};