#include "JY901.h"
#include "config.h"
#include "string.h"
xSemaphoreHandle xSemaphore_imu = NULL;
Imu imu = Imu(0);
CJY901 ::CJY901()
{
	ucDevAddr = 0x50;
}
void CJY901::StartIIC()
{
	ucDevAddr = 0x50;
	Wire.begin(IIC_SDA, IIC_SCL, 400000);
}
void CJY901::StartIIC(unsigned char ucAddr)
{
	ucDevAddr = ucAddr;
	Wire.begin(IIC_SDA, IIC_SCL, 400000);
}

void CJY901 ::CopeSerialData(unsigned char ucData)
{
	static unsigned char ucRxBuffer[250];
	static unsigned char ucRxCnt = 0;

	ucRxBuffer[ucRxCnt++] = ucData;
	if (ucRxBuffer[0] != 0x55)
	{
		ucRxCnt = 0;
		return;
	}
	if (ucRxCnt < 11)
	{
		return;
	}
	else
	{
		switch (ucRxBuffer[1])
		{
		case 0x50:
			memcpy(&stcTime, &ucRxBuffer[2], 8);
			break;
		case 0x51:
			memcpy(&stcAcc, &ucRxBuffer[2], 8);
			break;
		case 0x52:
			memcpy(&stcGyro, &ucRxBuffer[2], 8);
			break;
		case 0x53:
			memcpy(&stcAngle, &ucRxBuffer[2], 8);
			break;
		case 0x54:
			memcpy(&stcMag, &ucRxBuffer[2], 8);
			break;
		case 0x55:
			memcpy(&stcDStatus, &ucRxBuffer[2], 8);
			break;
		case 0x56:
			memcpy(&stcPress, &ucRxBuffer[2], 8);
			break;
		case 0x57:
			memcpy(&stcLonLat, &ucRxBuffer[2], 8);
			break;
		case 0x58:
			memcpy(&stcGPSV, &ucRxBuffer[2], 8);
			break;
		case 0x59:
			memcpy(&stcSQ, &ucRxBuffer[2], 8);
			break;
		}
		ucRxCnt = 0;
	}
}
void CJY901::readRegisters(unsigned char deviceAddr, unsigned char addressToRead, unsigned char bytesToRead, char *dest)
{
	int i;
	Wire.beginTransmission(deviceAddr);
	Wire.write(addressToRead);
	Wire.endTransmission(false); // endTransmission but keep the connection active

	Wire.requestFrom(deviceAddr, bytesToRead); // Ask for bytes, once done, bus is released by default

	while ((i = Wire.available()) < bytesToRead)
	{
		Serial.println(i);
	} // Hang out until we get the # of bytes we expect

	for (int x = 0; x < bytesToRead; x++)
		dest[x] = Wire.read();
}
void CJY901::writeRegister(unsigned char deviceAddr, unsigned char addressToWrite, unsigned char bytesToRead, char *dataToWrite)
{
	Wire.beginTransmission(deviceAddr);
	Wire.write(addressToWrite);
	for (int i = 0; i < bytesToRead; i++)
		Wire.write(dataToWrite[i]);
	Wire.endTransmission(); // Stop transmitting
}

short CJY901::ReadWord(unsigned char ucAddr)
{
	short sResult;
	readRegisters(ucDevAddr, ucAddr, 2, (char *)&sResult);
	return sResult;
}
void CJY901::WriteWord(unsigned char ucAddr, short sData)
{
	writeRegister(ucDevAddr, ucAddr, 2, (char *)&sData);
}
void CJY901::ReadData(unsigned char ucAddr, unsigned char ucLength, char chrData[])
{
	readRegisters(ucDevAddr, ucAddr, ucLength, chrData);
}

void CJY901::GetTime()
{
	readRegisters(ucDevAddr, 0x30, 8, (char *)&stcTime);
}
void CJY901::GetAcc()
{
	readRegisters(ucDevAddr, AX, 6, (char *)&stcAcc);
}
void CJY901::GetGyro()
{
	readRegisters(ucDevAddr, GX, 6, (char *)&stcGyro);
}

void CJY901::GetAngle()
{
	readRegisters(ucDevAddr, Roll, 6, (char *)&stcAngle);
}
void CJY901::GetMag()
{
	readRegisters(ucDevAddr, HX, 6, (char *)&stcMag);
}
void CJY901::GetPress()
{
	readRegisters(ucDevAddr, PressureL, 8, (char *)&stcPress);
}
void CJY901::GetDStatus()
{
	readRegisters(ucDevAddr, D0Status, 8, (char *)&stcDStatus);
}
void CJY901::GetLonLat()
{
	readRegisters(ucDevAddr, LonL, 8, (char *)&stcLonLat);
}
void CJY901::GetGPSV()
{
	readRegisters(ucDevAddr, GPSHeight, 8, (char *)&stcGPSV);
}
void CJY901::GetQ()
{
	readRegisters(ucDevAddr, Q0, 8, (char *)&stcSQ);
}

void CJY901::writeRegister_ser(uint8_t add, unsigned short data)
{
	Serial1.write(0xff);
	Serial1.write(0xaa);
	Serial1.write(add);
	uint8_t *p = (uint8_t *)&data;
	Serial1.write(p[1]);
	Serial1.write(p[0]);
}

Imu::Imu(uint8_t mode = 0) : CJY901()
{
	useIIC = mode;
	useSerial = !mode;
}
void taskImu(void *pvParameters)
{
	// Imu *imu = (Imu *)pvParameters;
	// float a[3], g[3], m[3];

	while (1)
	{

		xSemaphoreTake(xSemaphore_imu, portMAX_DELAY);
		imu.update();
		imu.getAcc();
		imu.getGyro();
		imu.getEuler();

		xSemaphoreGive(xSemaphore_imu);
		// Serial.printf("angle:%f,%f,%f\n", imu.roll, imu.pitch, imu.yaw);
		vTaskDelay(5);
	}
}
void Imu::init()
{
	if (useIIC)
	{
		CJY901::StartIIC();
		Serial.println("[IMU] CJY901 init in IIC mode");
	}
	else if (useSerial)
	{
		Serial.begin(115200);
		Serial1.begin(230400, SERIAL_8N1, 5, 6); // car_atr

		// CJY901::writeRegister_ser(0x01, 4);

		uint8_t cmd_buff3[] = {0xff, 0xaa, 0x01, 0x04, 0x00};

		Serial1.write(cmd_buff3, 5);
		delay(20);

		Serial.println("[IMU] CJY901 init in Serial mode");
	}
	xSemaphore_imu = xSemaphoreCreateMutex();
	BaseType_t xReturned = xTaskCreatePinnedToCore(
		taskImu,   /* Function to implement the task */
		"taskImu", /* Name of the task */
		4096,	   /* Stack size in words */
		NULL,	   /* Task input parameter */
		5,		   /* Priority of the task */
		NULL,	   /* Task handle. */
		0);		   /* Core where the task should run */
	if (xReturned == pdPASS)
	{
		Serial.println("[OS] taskImu created [OK]");
	}
	else
	{
		Serial.println("[OS] taskImu created [FAIL]]");
	}
	// delay(1000);
	// Serial.println("imu init");
}

void Imu::update()
{

	if (useIIC)
	{
		GetTime();
		GetAcc();
		GetGyro();
		GetAngle();
		GetMag();
		//  GetPress();
		//  GetDStatus();
		//  GetLonLat();
		//  GetGPSV();
		//  GetQ();
	}
	else if (useSerial)
	{
		while (Serial1.available())
		{
			CopeSerialData(Serial1.read());
		}
	}
}

void Imu::getAcc(float *x, float *y, float *z)
{
	*y = stcAcc.a[0] / 32768.0 * 8;
	*x = stcAcc.a[1] / 32768.0 * 8;
	*z = stcAcc.a[2] / 32768.0 * 8;
}
void Imu::getGyro(float *x, float *y, float *z)
{
	*y = stcGyro.w[0] / 32768.0 * 2000 / 180 * 3.1415926;
	*x = stcGyro.w[1] / 32768.0 * 2000 / 180 * 3.1415926;
	*z = stcGyro.w[2] / 32768.0 * 2000 / 180 * 3.1415926;
}

void Imu::getEuler(float *x, float *y, float *z)
{
	*x = stcAngle.Angle[0] / 32768.0 * 3.1415926;
	*y = stcAngle.Angle[1] / 32768.0 * 3.1415926;
	*z = stcAngle.Angle[2] / 32768.0 * 3.1415926;
}
void Imu::getMag(float *x, float *y, float *z)
{
	*x = stcMag.h[0];
	*y = stcMag.h[1];
	*z = stcMag.h[2];
}
void Imu::getAcc()
{
	getAcc(&this->accx, &this->accy, &this->accz);
}
void Imu::getEuler()
{

	getEuler(&this->roll, &this->pitch, &this->yaw);
}
void Imu::getGyro()
{
	this->lgyrox = this->gyrox;
	this->lgyroy = this->gyroy;
	this->lgyroz = this->gyroz;
	getGyro(&this->gyrox, &this->gyroy, &this->gyroz);
}
void Imu::getMag()
{
	getMag(&this->magx, &this->magy, &this->magz);
}

typedef struct Quaternion
{
	double w, x, y, z;
} Quaternion;

// 四元数乘法函数
Quaternion quaternion_multiply(Quaternion q1, Quaternion q2)
{
	Quaternion result;
	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
	result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
	return result;
}
void Imu::getQ(float *w, float *x, float *y, float *z)
{
	*z = stcSQ.q0 / 32768.0;
	*y = stcSQ.q1 / 32768.0;
	*x = stcSQ.q2 / 32768.0;
	*w = stcSQ.q3 / 32768.0;
}

// CJY901 CJY901 = JY901();
