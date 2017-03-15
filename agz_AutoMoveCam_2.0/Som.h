#include<iostream>
#include<vector>
#include<string>
#include<time.h>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/opencv.hpp>

class SOM{
private:
	int width,height;
	cv::UMat image;
	/* 構造体 Neuron を定義 */
	typedef struct {
		int id;					/*ニューロンID*/
		cv::Point2f p;			/*ニューロン座標*/
		std::vector<int>link;	/*隣接情報*/
		cv::Point2f weight;		/*ニューロンの荷重*/
	} Neuron;

public:
	SOM(int width, int height, cv::UMat &src);
	/**
	* @fn
	* SOM(自己組織化マップ)の生成を行う関数
	* @brief 引数で与えられたサイズのSOMを作成する
	* @param (int w) SOMのサイズ(横)
	* @param (int h) SOMのサイズ(縦)
	* @return void
	* @sa     -
	* @detail -
	*/
	void Init(cv::Mat &src);

	/**
	* @fn
	* 画像の初期化を行う関数
	* @brief （白色で画像を初期化する）
	* @param (cv::Mat &img)画像を参照で受ける
	* @return void
	* @sa     -
	* @detail -
	*/
	void InitImg(cv::Mat &img);

	/**
	* @fn
	* ニューロン通しを直線で結ぶ関数
	* @brief 隣接ニューロンの座標から直線を引く
	* @param (std::vector<Neuron> &neu)ニューロンの情報(構造体)を参照
	* @param (cv::Mat &img)画像を参照で受ける
	* @return void
	* @sa     -
	* @detail -
	*/
	void edgeline(std::vector<Neuron> &neu, cv::Mat &img);

	/**
	* @fn
	* SOMの処理を行う関数
	* @brief マップの作成、描画
	* @param (int w) 画像の横幅
	* @param (int h) 画像の縦幅
	* @param (std::vector<Neuron> &neu)ニューロンの情報(構造体)を参照
	* @return void
	* @sa     -
	* @detail -
	*/
	void som(int w, int h, std::vector<Neuron> &som, cv::Mat &src, cv::Mat &origin);



	/**
	* @fn
	* SOMを表示
	* @brief 大小比較
	* @param (int index)発火ニューロンのインデックス
	* @param (std::vector<Neuron> &def) ニューロンの情報(構造体)を参照
	* @return void
	* @sa     -
	* @detail -
	*/
	void showSOM(int index, std::vector<Neuron> &def);

	void Imgproc(cv::Mat &src, cv::Mat &dst);

	std::vector<cv::Point2f> storePoint(cv::Mat &img);
};


bool cmp(int A, int B);
