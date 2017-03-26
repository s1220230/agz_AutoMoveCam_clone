#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "SOM.h"

class Control{
private:
	int height;
	int width;

	// ロボットの現在位置
	cv::Point2i nowPoint;
	// ロボットの動作
	std::string action;
	// 内外判定結果
	std::string out;

	struct target{
		// ターゲット座標
		cv::Point2i point;
		// ターゲット番号
		int n;
		std::vector<int> neighbor;
	};

	// すべてのターゲット
	std::vector<target> allTarget;

	//変換されたすべてのターゲット
	std::vector<target> allTransformedTraget;

	// 次に向かうターゲット
	std::vector<target>::iterator nowTarget_itr;

	// ヒートマップ用配列 ロボットの訪問回数を格納する
	int** small_area;

	//@comment カラーバー、ヒートマップ結合用画像生成
	cv::Mat concat_img = cv::Mat(cv::Size(700, 800), CV_8UC3, cv::Scalar(255, 255, 255));


public:
	// Constructor
	Control(int width, int height);

	// ターゲットの更新
	bool is_updateTarget(void);

	// ロボットの動作決定
	int robot_action(cv::Point2i Previous);

	// ターゲットエリアの訪問回数の更新
	int target_count(void);

	// 小領域区間の訪問回数の更新
	cv::Point2i area_count(void);

	// 内外判定
	void is_out(void);

	// プロット
	void plot_target(cv::UMat &img, cv::Point2i Previous);

	void plot_transform_target(cv::UMat &img, cv::Point2i Previous, cv::Mat H);

	//@comment ヒートマップ作成
	void heatmap(cv::Point2i pos, cv::Mat *img, cv::Mat *bar);

	//	set function
	void set_target(SOM som);
	void set_point(cv::Point2i p);


	// get function
	int get_target(void);

	std::vector<int> get_nowTargetArea(void);
};