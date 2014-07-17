#pragma once

#include "stdafx.h"
#include "LJV7_IF.h"
#include "LJV7_ErrorCode.h"
#include <iostream>
#include <fstream>
using namespace std;


class CLJV7000DLL
{
	private:



		CLJV7000DLL()
		{
			ethernet_config.abyIpAddress[0] = 192;
			ethernet_config.abyIpAddress[1] = 168;
			ethernet_config.abyIpAddress[2] = 0;
			ethernet_config.abyIpAddress[3] = 11;
			ethernet_config.wPortNo = 24691;
			MAX_PROFILE_COUNT= 3200;
		}



		~CLJV7000DLL()
		{
			Exit();
		}


		LJV7IF_MEASURE_DATA* data;
		ofstream of;
		LONG nDeviceID;
		LJV7IF_ETHERNET_CONFIG ethernet_config;
		LONG MAX_PROFILE_COUNT ;

	public:

		void SetIPAddress(BYTE IpAddress[4],WORD port)
		{
			for( int i = 0; i< 4; i++)
			{
				ethernet_config.abyIpAddress[i] = IpAddress[i];
			}
				ethernet_config.wPortNo = port;
		}


		// must initialize with device ID 
		BOOL Initialization( LONG DeviceID=0)
		{
			// need to configure the LJV7_IF.lib 
			nDeviceID = DeviceID;
			//memset(data, 40 * 16 * sizeof(LJV7IF_MEASURE_DATA), 0);
			LONG result = LJV7IF_Initialize();
			return true;
		}

		BOOL Trigger()
		{
			LONG result = LJV7IF_Trigger(nDeviceID);
			return true;

		}

		BOOL OpenUSB()
		{
			LONG result = LJV7IF_UsbOpen(nDeviceID);
			return true;
		}

		BOOL OpenEthernet()
		{
			LONG result = LJV7IF_EthernetOpen(nDeviceID,&ethernet_config);
			return true;
		}

		BOOL GetStorageData()
		{
			// TODO: Add your control notification handler code here
			LJV7IF_GET_STORAGE_REQ *pReq = (LJV7IF_GET_STORAGE_REQ *)malloc(sizeof(LJV7IF_GET_STORAGE_REQ));
			LJV7IF_STORAGE_INFO *pInfo = (LJV7IF_STORAGE_INFO*)malloc(sizeof(LJV7IF_STORAGE_INFO));
			LJV7IF_GET_STORAGE_RSP *pRes = (LJV7IF_GET_STORAGE_RSP*)malloc(sizeof(LJV7IF_GET_STORAGE_RSP));

		
			pReq->dwSurface = 0;
			pReq->dwStartNo = 0;
			pReq->dwDataCnt = 8000;

		
			DWORD dwDataSize = sizeof(LJV7IF_MEASURE_DATA) * 8000;
			DWORD *pdwData = (DWORD*)malloc(dwDataSize);

			LONG result = LJV7IF_GetStorageData(0, pReq, pInfo, pRes, pdwData, dwDataSize );

			LJV7IF_MEASURE_DATA *pData = (LJV7IF_MEASURE_DATA *)pdwData;
			for(int i=0; i<200; ++i)
			{
				for(int j=0; j<40; ++j)
				{
					of << pData->fValue << '\t';
					pData++;
				}
				of << endl;
			}

		 return true;
		}

		BOOL GetMeasureValue(int Length = 16)
		{
			data = new LJV7IF_MEASURE_DATA [Length];
			memset(data, Length*sizeof(LJV7IF_MEASURE_DATA), 0);
			LONG result = LJV7IF_GetMeasurementValue(nDeviceID,data);

			return true;
		}

		BOOL Exit()
		{
			LONG result = LJV7IF_CommClose(nDeviceID);
				result = LJV7IF_Finalize();
			return true;
		}

		BOOL StartHighSpeedCommunication()
		{
			return true;
		}

		BOOL GetHighSpeedModeProfile()
		{
			LJV7IF_GET_PROFILE_REQ req;
				req.byTargetBank = 0x00; // 0x00 active 0x01 inactive ProfileBank.Active;
				req.byPosMode = 0x00; // 0x00 current 0x01 oldest 0x10 spec;
				req.dwGetProfNo = 0;
				req.byGetProfCnt = 10;
				req.byErase = 0;

				LJV7IF_GET_PROFILE_RSP rsp ;
				LJV7IF_PROFILE_INFO profileInfo;

				int profileDataSize = MAX_PROFILE_COUNT + 
					(sizeof(LJV7IF_PROFILE_HEADER) + sizeof(LJV7IF_PROFILE_FOOTER) )/ sizeof(int);
				int* receiveBuffer = new int[profileDataSize * req.byGetProfCnt];

		
					// Get profiles.
			/*	LONG result = LJV7IF_GetProfile(nDeviceID, &req, &rsp, &profileInfo,receiveBuffer,profileDataSize); 

					// Output profile data
					List<ProfileData> profileDatas = new List<ProfileData>();
					int unitSize = ProfileData.CalculateDataSize(profileInfo);
					for (int i = 0; i < rsp.byGetProfCnt; i++)
					{
						profileDatas.Add(new ProfileData(receiveBuffer, unitSize * i, profileInfo));
					}

					progressForm.Status = Status.Saving;
					progressForm.Refresh();

					// Save file
					SaveProfile(profileDatas, _txtSavePath.Text);
				}*/
		}
};