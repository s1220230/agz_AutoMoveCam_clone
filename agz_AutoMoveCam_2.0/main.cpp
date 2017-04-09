#include "Xbee_com.h"
#include "Img_proc.h"
#include "Control.h"
#include <conio.h>
#include <fstream>
#include <time.h>

#define GRAVITY 1      //�摜���̗̈� : 0  ���ڗ̈� : 1
#define CAM_ID 1	   //�J����ID
const LPCSTR com = "COM5";		//COM�|�[�g��
std::vector<cv::Point2f> Pos;	//���c�̎l���̓_
std::vector<cv::Point2f> Pos2;

cv::Point2i target, P0[5] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, P1 = { 0, 0 };
cv::Point2i pre_point; // @comment Point�\����<int�^>
int action;			   //���{�b�g�̓���ϐ� 1:�O�i 2:�E���� 4:��

SOM s = SOM();
SOM s2 = SOM();

//cv::Mat sample_img = cv::imread("test2.JPG", 1);            //�e�X�g�摜�p�iSOM�j
//cv::Mat sample_img = cv::imread("rice.jpg", 1);            //�e�X�g�摜�p�iSOM�j
//if (!sample_img.data)exit(0);								//�e�X�g�摜�p�iSOM�j
//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //�e�X�g�摜�p�iSOM�j


//�f�[�^�o�͗pcsv�t�@�C�����쐬�֐��@
std::string setFilename(std::string str){
	time_t now = time(NULL);
	struct tm * pnow = localtime(&now);
	char time[32];
	std::string c = ".csv";
	std::string data = "./data/";

	//@comment sprintf���g����int�^��string�ɕϊ�
	sprintf(time, "%d_%d_%d_%d_%d", pnow->tm_year + 1900, pnow->tm_mon + 1,
		pnow->tm_mday, pnow->tm_hour, pnow->tm_min);

	return data + str + time + c; //@comment �t�@�C����
}

//�}�E�X�N���b�N�擾���W�ʒu�ɉ~���v���b�g
void drawPoint(cv::Mat* img, cv::Point2i point){
	cv::circle(*img, point, 8, cv::Scalar(0, 255, 0), -1, CV_AA);
	cv::imshow("getCoordinates", *img);
}

//���c�̈�̍��W�擾�p�֐�
void getCoordinates(int event, int x, int y, int flags, void* param)
{
	cv::Mat* image = static_cast<cv::Mat*>(param);
	static int count = 0;
	switch (event) {
	case CV_EVENT_LBUTTONDOWN://@comment ���N���b�N�������ꂽ��
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
		//4�_�ȏ�N���b�N����Ȃ��ꍇ�摜�̓�����4�_���w��
		if (count < 4){

			std::cout << std::endl;
			std::cout << "4�_�ȏ�w�肵�Ă�������" << std::endl<<std::endl;
			system("pause");
			exit(0);
		}
		cv::destroyAllWindows();
		break;
	}
}


//�摜���擾��,���c�̈��ݒ�
void setUp(LPCSTR com, HANDLE &hdl, Img_Proc &imp){
	int width=10, height=10;
	cv::Mat field;
	cv::UMat src_frame, dst_img;


	//std::cout << "���c�̑傫������͂��Ă�������(m)�P��" << std::endl;
	//std::cout << "�� : ";    std::cin >> width;
	//std::cout << "�c : ";    std::cin >> height;
	//std::cout << std::endl;

	width *= 100;
	height *= 100;

	//if (width < 300 || height < 300) //@comment 3x3(m)�ȏ�̗̈���w��
	//{
	//	std::cout << "�� �c�A�����ꂼ��R���ȏ���w�肵�Ă�������" << std::endl;
	//	std::cout << std::endl;
	//	system("PAUSE");
	//	exit(0);
	//}

	imp.setField(width, height);
	nm30_init();
	nm30_set_panorama_mode(1, 11); //@comment ����␳

	//@comment �n�߂̂P�O�t���[���͓ǂݔ�΂�
	for (int i = 0; i < 10; i++) {
		imp.getFrame().copyTo(src_frame);//@comment 1�t���[���擾
	}
	//if (!sample_img.data)exit(0);								//�e�X�g�摜�p�iSOM�j
	//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //�e�X�g�摜�p�iSOM�j
	//sample_img.copyTo(src_frame); //�e�X�g�摜�p�iSOM�j
	std::cout << "���c�̗̈���������玞�v���ɂȂ�悤�ɂS�_�N���b�N���Ă�������" << std::endl;

	//------------------���W�擾-----------------------------------------------
	//�摜������}�E�X��4 ~ 10�_���擾���̌�E�N���b�N����ƕϊ��������J�n����
	cv::namedWindow("getCoordinates");
	cv::imshow("getCoordinates", src_frame);
	src_frame.copyTo(field);
	cv::setMouseCallback("getCoordinates", getCoordinates, (void *)&field);

	cv::waitKey(0);
	cv::destroyAllWindows();

	//------------------�����ϊ�-----------------------------------------------

	imp.Perspective(src_frame, dst_img, Pos);

	Pos2.push_back(cv::Point(30, dst_img.rows - 30));
	Pos2.push_back(cv::Point(30, 30));
	Pos2.push_back(cv::Point(dst_img.cols - 30, 30));
	Pos2.push_back(cv::Point(dst_img.cols - 30, dst_img.rows - 30));

	cv::Point pt2[10]; //�C�ӂ�4�_��z��ɓ����
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
	cv::fillConvexPoly(ss2, pt2, Pos.size(), cv::Scalar(200, 200, 200));//���p�`��`��
	s2.Init2(ss2);
	cv::destroyAllWindows();
}

//���䃋�[�v
void Moving(HANDLE &arduino, Xbee_com &xbee, Img_Proc &imp){
	cv::Mat element = cv::Mat::ones(3, 3, CV_8UC1); //2�l�摜�c���p�s��
	cv::Mat heatmap_img(cv::Size(500, 500), CV_8UC3, cv::Scalar(255, 255, 255));
	int frameNum = 0;								//�t���[�����ێ��ϐ�
	cv::UMat src, dst, colorExtra, pImg, binari_2, copyImg, copyImg2;
	cv::Point2f sz = imp.getField();
	Control control(sz.x, sz.y);
	control.set_target(s2);
	char command = 's';
	int ypos;
	int ydef = 0;	//�␳�Ȃ��d�S���W�l
	int num = 0;	// �^�[�Q�b�g�̖K��񐔍X�V
	std::ofstream ofs(setFilename("Coordinate")); // @comment�@���{�b�g���W�pcsv

	//���{�b�g���Wcsv
	ofs << imp.getField().x << ", " << imp.getField().y << std::endl;
	ofs << "x��, y���i�␳�Ȃ��j, ypos�i�␳����j" << std::endl;

	////////////////////////////////////////////////////////////////
	//���{�b�g�F���o�e�X�g�p
	//int h_value = 180,h_value2 = 0,s_value = 70,v_value = 70;
	//cv::namedWindow("colorExt", 1);
	//cv::createTrackbar("Hmax", "colorExt", &h_value, 180);
	//cv::createTrackbar("Hmin", "colorExt", &h_value2, 180);
	//cv::createTrackbar("S", "colorExt", &s_value, 255);
	//cv::createTrackbar("V", "colorExt", &v_value, 255);
	///////////////////////////////////////////////////////////////

	std::cout << "a: �����|��" << std::endl;
	std::cout << "s: ��~" << std::endl;
	std::cout << "q �������̓E�B���h�E�E��́~�{�^�� : �I��" << std::endl;

	while (1){
		imp.getFrame().copyTo(src);
		//cv::resize(sample_img, sample_img, cv::Size(), 0.15, 0.15); //�e�X�g�摜�p�iSOM�j
		//sample_img.copyTo(src); //�e�X�g�摜�p�iSOM�j
		if (frameNum % 1 == 0){
			if (_kbhit()){
				command = _getch();
				if (command == 's') {
					xbee.sentManualCommand(byte(0x00), arduino);
					std::cout << "��~" << std::endl << std::endl;
				}
				if (command == 'q') {
					xbee.sentManualCommand(byte(0x01), arduino);
					std::cout << "�I�� " << std::endl << std::endl;
					cv::destroyAllWindows();
					ofs.close(); //@comment �t�@�C���X�g���[���̉��
					break;
				}
				if (command == 'a') {
					xbee.sentManualCommand(byte(0x01), arduino);
					std::cout << "�����|��" << std::endl << std::endl;
				}
			}
			//@comment �摜�����T�C�Y(�傫������ƃf�B�X�v���C�ɓ����Ȃ�����)
			src.copyTo(copyImg);
			cv::warpPerspective(src, dst, imp.getPersMat(), cv::Size(src.cols, src.rows), CV_INTER_LINEAR);
			dst.copyTo(copyImg2);

			//@comment hsv�𗘗p���ĐԐF�𒊏o
			//���͉摜�A�o�͉摜�A�ϊ��Ah�ŏ��l�Ah�ő�l�As�ŏ��l�As�ő�l�Av�ŏ��l�Av�ő�l h:(0-180)���ۂ�1/2
			//imp.colorExtraction(dst, colorExtra, CV_BGR2HSV, h_value, h_value2, s_value, 255, v_value, 255);
			imp.colorExtraction(dst, colorExtra, CV_BGR2HSV, 160, 10, 70, 255, 70, 255);
			colorExtra.copyTo(pImg);
			cv::cvtColor(colorExtra, colorExtra, CV_BGR2GRAY);//@comment �O���[�X�P�[���ɕϊ�

			//----------------------��l��-----------------------------------------------
			cv::threshold(colorExtra, binari_2, 0, 255, CV_THRESH_BINARY);
			cv::dilate(binari_2, binari_2, element, cv::Point(-1, -1), 3); //�c������3�� �Ō�̈����ŉ񐔂�ݒ�

			//---------------------�ʐόv�Z,�d�S�擾-----------------------------------------------
			//�擾�����̈�̒��ň�Ԗʐς̑傫�����̂�ΏۂƂ��Ă��̑Ώۂ̏d�S�����߂�B
			cv::Point2i point = imp.serchMaxArea(binari_2, pImg);
			if (!GRAVITY)
			{
				point = imp.calculate_center(binari_2);//@comment moment�Ŕ��F�����̏d�S�����߂�
				std::cout << "posion: " << point.x << " " << point.y << std::endl;//@comment �d�S�_�̕\��
			}

			if (point.x != 0) {
				ypos = sz.y - (point.y + 6 * ((1000 / point.y) + 1));
				ydef = sz.y - point.y;//@comment �␳�Ȃ����d�S
				//std::cout << point.x << " " << ypos << std::endl; //@comment �ϊ��摜���ł̃��{�b�g�̍��W(�d�S)
				ofs << point.x << ", " << ydef << ", " << ypos << std::endl; //@comment �ϊ�
			}

			//---------------------���{�b�g�̓���擾------------------------------------
			//if (frame % 2 == 0){
			P1 = cv::Point2f(point.x, sz.y - ydef);
			if (P1.x != 0 && P1.y != 0) {
				// �^�[�Q�b�g�̍X�V
				if (control.is_updateTarget()){
					//std::cout << "target number : " << control.get_target() << std::endl;
				}
				// ���݂̃��{�b�g�̈ʒu���̍X�V
				control.set_point(P1);

				// ���{�b�g�̓��쌈��
				action = control.robot_action(P0[4]);

				// �^�[�Q�b�g�̖K��񐔍X�V
				//num = control.target_count();

				control.heatmap(control.area_count(), &heatmap_img, &imp.makeColorbar());
				// ���O����
				control.is_out();
				for (int i = 1; i < 5; i++){
					P0[i] = P0[i - 1];
				}
			}

			else{
				action = 0;
			}
			P0[0] = P1;
			//} //���{�b�g�̓���擾

			if (command == 'a'){
				xbee.sentAigamoCommand(action, arduino);
			}
			////std::cout << "cmd " << int(command) << std::endl;
			cv::UMat test, test2;
			src.copyTo(test);
			dst.copyTo(test2);

			cv::warpPerspective(test2, test, imp.getInvPerse(), cv::Size(test.cols, test.rows), CV_INTER_LINEAR);


			//-------------------�d�S�_�̃v���b�g----------------------------------------- 
			if (!point.y == 0) { //@comment point.y == 0�̏ꍇ��exception���N����( 0���Z )
				
				circle(copyImg, imp.calcHomoPoint(cv::Point2f(point)), 8, cv::Scalar(0, 0, 255), -1, CV_AA);
				circle(dst, cv::Point(point.x, point.y), 8, cv::Scalar(0, 0, 255), -1, CV_AA);
			}

			//------------------�^�[�Q�b�g�̃v���b�g--------------------------------------

			control.plot_target(dst, P0[4]);
			control.plot_transform_target(copyImg, P0[4], imp.getInvPerse());

			//------------------�}�X, ���i�̈�̃v���b�g--------------------------------------

			s2.showSOM3(dst,control.get_nowTargetArea());
			s2.showSOM2(copyImg, imp.getInvPerse(),control.get_nowTargetArea());

			//---------------------�\������----------------------------------------------

			cv::imshow("dst_image", dst);//@comment �o�͉摜
			cv::imshow("copyImg", copyImg);
			
			cv::waitKey(1);

			//@comment "q"����������v���O�����I��
			//if (src.empty() || cv::waitKey(50) == 113)
			//{
			//	cv::destroyAllWindows();
			//	ofs.close(); //@comment �t�@�C���X�g���[���̉��
			//	break;
			//}
		}
		frameNum++;
	} 
	ofs.close(); //@comment �t�@�C���X�g���[���̉��
}

//���{�b�g��������
void AutoMove(){
	HANDLE hdl;							//COM�|�[�g�ʐM�n���h��
	Img_Proc imp = Img_Proc(CAM_ID);    //�摜�����p�N���X
	Xbee_com xbee = Xbee_com(com, hdl); 	//xbee�ʐM�̏�����
	setUp(com, hdl, imp);				//�����Z�b�g�A�b�v
	Moving(hdl, xbee, imp);				//�������䃋�[�v
}

int main(int argc, char *argv[]){
	AutoMove();
	return 0;
}