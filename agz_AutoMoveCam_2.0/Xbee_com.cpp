#include "Xbee_com.h"

//xbeeの初期設定
Xbee_com::Xbee_com(LPCSTR com, HANDLE &arduino){
	openCOM(com, arduino);

	//送受信バッファ初期化
	if (!SetupComm(arduino, 1024, 1024)) {
		printf("SET UP FAILED\n");
		CloseHandle(arduino);
		system("PAUSE");
		exit(0);
	}

	if (!PurgeComm(arduino, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
		printf("CLEAR FAILED\n");
		CloseHandle(arduino);
		exit(0);
	}

	//基本通信条件の設定
	DCB dcb;
	GetCommState(arduino, &this->dcb);
	this->dcb.DCBlength = sizeof(DCB);
	this->dcb.BaudRate = 57600;
	this->dcb.fBinary = TRUE;
	this->dcb.ByteSize = 8;
	this->dcb.fParity = NOPARITY;
	this->dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(arduino, &this->dcb)) {
		printf("SetCommState FAILED\n");
		CloseHandle(arduino);
		system("PAUSE");
		exit(0);
	}

}

//モードの切り替え関数
void Xbee_com::sentManualCommand(byte command, HANDLE &arduino){
	bool Ret;
	DWORD dwSendSize;
	byte checksum = 0;

	//パケット生成
	byte requestPacket[] = { byte(0x7E), byte(0x00), byte(0x1A), byte(0x10), byte(0x01),
		robotAddr[0], robotAddr[1], robotAddr[2], robotAddr[3],
		robotAddr[4], robotAddr[5], robotAddr[6], robotAddr[7],
		byte(0xFF), byte(0xFE), byte(0x00), byte(0x00), A, G, S, C, F, A, T, A, command, A, G, E, byte(0x00) };

	//チェックサムの計算
	for (int i = 3; i < 29; i++) {
		checksum += requestPacket[i];
	}
	checksum = 0xFF - (checksum & 0x00FF);
	requestPacket[29] = byte(checksum);

	//パケットの送信
	Ret = WriteFile(arduino, requestPacket, sizeof(requestPacket), &dwSendSize, NULL);
	if (!Ret) {
		printf("SEND FAILED\n");
		CloseHandle(arduino);
		system("PAUSE");
		exit(0);
	}
}

//動作パターン変更関数
void Xbee_com::sentAigamoCommand(int command, HANDLE &arduino) {
	bool Ret;
	DWORD dwSendSize;
	DWORD dwErrorMask;
	byte checksum = 0;

	//パケット生成
	byte requestPacket[] = { byte(0x7E), byte(0x00), byte(0x1F), byte(0x10), byte(0x01),
		robotAddr[0], robotAddr[1], robotAddr[2], robotAddr[3],
		robotAddr[4], robotAddr[5], robotAddr[6], robotAddr[7],
		byte(0xFF), byte(0xFE), byte(0x00), byte(0x00), A, G, S,
		M, F, A, T, A, L, 1, lPwm[byte(command)], R, 1, rPwm[byte(command)], A, G, E, byte(0x00) };

	//チェックサムの計算
	for (int i = 3; i < 34; i++) {
		checksum += requestPacket[i];
	}
	checksum = 0xFF - (checksum & 0x00FF);
	requestPacket[34] = byte(checksum);

	//パケットの送信
	Ret = WriteFile(arduino, requestPacket, sizeof(requestPacket), &dwSendSize, NULL);
	if (!Ret) {
		printf("SEND FAILED\n");
		CloseHandle(arduino);
		system("PAUSE");
		exit(0);
	}
}

//COMポートオープン
void Xbee_com::openCOM(LPCSTR com, HANDLE &arduino){
	arduino = CreateFile(com, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (arduino == INVALID_HANDLE_VALUE){
		std::cout << "COMポートが開きません." << std::endl;
		std::cout << "COMポートをチェックし, " << std::endl;
		std::cout << "正しいポートを指定してください." << std::endl;
		system("PAUSE");
		exit(0);
	}
}

//COMポートクローズ
void Xbee_com::closeCOM(HANDLE &arduino){
	CloseHandle(arduino);
}