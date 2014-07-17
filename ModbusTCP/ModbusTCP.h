
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include "stdint.h"

#pragma comment(lib, "Ws2_32.lib")


// because ADU maximum is 261 bytes
#define MAX_LENGTH_ONE_TIME 100

class modbus_tcp 
{
	private:
		char ip_adrs[100];
		uint16_t unit;
		uint16_t port;
		SOCKET s;
	    fd_set fds;
	    timeval tv;
		struct sockaddr_in server;
		WSADATA wd;
	
	public:
		char error_msg[200];
		modbus_tcp()
		{
			strcpy_s(ip_adrs,"192.168.1.20");
			port = 502; 
			FD_ZERO(&fds);  
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			unit = 0;
		}
		~modbus_tcp()
		{
			Close();
		}
		bool InitialSocket()
		{
		  // initialize WinSock
		
		  if (WSAStartup(0x0101, &wd))
		  {
			sprintf_s(error_msg,"cannot initialize WinSock\n");
			return false;
		  }
		  return true;
		}

		void Close()
		{
			  // close down
			  closesocket(s);
			  WSACleanup();
		}
		bool Connect()
		{
			  if(!InitialSocket()) 
				  return false;
			
			  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			  server.sin_family = AF_INET;
			  server.sin_port = htons(port); // ASA standard port
			  server.sin_addr.s_addr = inet_addr(ip_adrs);
			  int i;
			  i = connect(s, (sockaddr *)&server, sizeof(sockaddr_in));
			  if (i<0)
			  {
				sprintf_s(error_msg,"connect - error %d\n",WSAGetLastError());
				closesocket(s);
				WSACleanup();
				return false;
			  }
			  return true;
		}
		bool GetPermissionToSend()
		{
			 int i;
			  // wait for permission to send
			  FD_SET(s, &fds);
			  i = select(32, NULL, &fds, NULL, &tv); // write
			  if (i<=0)
			  {
				sprintf_s(error_msg, "select - error %d\n",WSAGetLastError());
				closesocket(s);
				WSACleanup();
				return false;
				}
			  return true;
		}

		bool GetResponse()
		{
					 // wait for response
		  FD_SET(s, &fds);
		  int i = select(32, &fds, NULL, NULL, &tv); //read
		  if (i<=0)
		  {
			 strcpy_s(error_msg,("no TCP response received\n"));
			closesocket(s);
			WSACleanup();
			return false;
		  }

		  return true;
		}


		bool ReadMultipleRegisters(uint16_t reg_no, uint16_t num_regs,uint16_t* value)
		{
			if (num_regs < MAX_LENGTH_ONE_TIME)
			{
				return ReadMultipleRegistersOneTime( reg_no, num_regs, value);
			}
			else
			{
				int n = num_regs/MAX_LENGTH_ONE_TIME;
				int m = num_regs%MAX_LENGTH_ONE_TIME;

				int i;

				for( i = 0 ;i< n ;i++)
				{
					if(!ReadMultipleRegistersOneTime( reg_no + i*MAX_LENGTH_ONE_TIME, MAX_LENGTH_ONE_TIME, value + i*MAX_LENGTH_ONE_TIME))
						return false;
				}

				if( m > 0 )
				{
					if(!ReadMultipleRegistersOneTime( reg_no + n*MAX_LENGTH_ONE_TIME, m, value + n*MAX_LENGTH_ONE_TIME))
						return false;
				}
					return true;
								
			}


		}

		bool ReadMultipleRegistersOneTime(uint16_t reg_no, uint16_t num_regs,uint16_t* value)
		{
			if(!GetPermissionToSend()) return false;
				 int i;	

				 byte* obuf = new byte [12];
				 byte* ibuf = new byte [9+2*num_regs];
				  obuf[0] = 0;
				  obuf[1] = 0;
				  obuf[2] = 0;
				  obuf[3] = 0;
				  
				  obuf[4] = 0;
				  obuf[5] = 6;
				  obuf[6] = unit;
				  obuf[7] = 3;
				  obuf[8] = reg_no >> 8;
				  obuf[9] = reg_no & 0xff;
				  obuf[10] = num_regs >> 8;
				  obuf[11] = num_regs & 0xff;

			  // send request
			  i = send(s, (char*) obuf, 12, 0);
			  if (i<12)
			  {
				  strcpy_s(error_msg,"failed to send all 12 chars\n");
				  return false;
			  }
			  if(!GetResponse())
			  {
				  return false;
			  }

			   // read response
			 i = recv(s, (char*)ibuf, 9+2*num_regs, 0);
			  if (i<9)
			  {
				if (i==0)
				{
				  sprintf_s(error_msg,"Read unexpected close of connection at remote end\n");
				}
				else
				{
				  sprintf_s(error_msg,"Read response was too short - %d chars\n", i);
				}
				return false;
			  }
			  else if (ibuf[7] & 0x80)
			  {
				sprintf_s(error_msg,"Read MODBUS exception response - type %d\n", ibuf[8]);
				return false;
			  }
			  else if (i != (9+2*num_regs))
			  {
				sprintf_s(error_msg,"Read incorrect response size is %d expected %d\n",i,(9+2*num_regs));
				return false;
			  }
			  else
			  {
				for (i=0;i<num_regs;i++)
				{
				  value[i] = (ibuf[9+i+i]<<8) + ibuf[10+i+i];
				 }
			  }
			  delete [] obuf;
			  delete [] ibuf;

			  return true;

		}


		
		bool WriteMultipleRegisters(uint16_t reg_no, uint16_t num_regs,uint16_t* value)
		{
			unsigned short MAX_WRITE = MAX_LENGTH_ONE_TIME;

			if (num_regs <= MAX_WRITE)
			{
				return WriteMultipleRegistersOneTime( reg_no, num_regs, value);
			}
			else
			{
				int n = num_regs/MAX_WRITE;
				int m = num_regs%MAX_WRITE;

				int i;

				for( i = 0 ;i< n ;i++)
				{
					if(!WriteMultipleRegistersOneTime( reg_no + i*MAX_WRITE, MAX_WRITE, value + i*MAX_WRITE))
						return false;
				}
				if( m > 0 ) 
				{
					if(!WriteMultipleRegistersOneTime( reg_no + n*MAX_WRITE, m, value + n*MAX_WRITE))
						return false;
				}
					return true;
								
			}
		}


		bool WriteMultipleRegistersOneTime(uint16_t reg_no, uint16_t num_regs,uint16_t* values)
		{
			if(!GetPermissionToSend()) return false;

			 uint16_t bytecount = 7 + num_regs*2;
     		  byte* obuf = new byte [13+num_regs*2];
	     	  byte* ibuf = new byte [12];
				 
				 
				 obuf[0] = 0;
				 obuf[1] = 0;
				 obuf[2] = 0;
				 obuf[3] = 0;
				 obuf[4] = bytecount >> 8;
				 obuf[5] = bytecount & 0xff;
				 obuf[6] = unit;			      
				 obuf[7] = 16;
				 obuf[8] = reg_no >> 8;
				 obuf[9] = reg_no & 0xff;
				 obuf[10] = num_regs >> 8;
				 obuf[11] = num_regs & 0xff;
				 obuf[12] = num_regs*2;

				  for(int i=0;i<num_regs;i++)
				  {
					  obuf[13+2*i] = values[i] >> 8;
					  obuf[13+2*i+1] = values[i] & 0xff;

				  }

			  // send request
			  int iResult = send(s, (char*) obuf, 13+num_regs*2, 0);

			  if ( iResult  == SOCKET_ERROR) {
				sprintf_s(error_msg,"Write send failed with error: %d\n", WSAGetLastError());
				Close();
				return false;
			 }
			  if (iResult<13+num_regs*2)
			  {
				  strcpy_s(error_msg,"Write failed to send all chars\n");
				  return false;
			  }
			  if(!GetResponse())
			  {
				  return false;
			  }

			   // read response
			  iResult = recv(s, (char*) ibuf, 12, 0);


			  if (iResult<9)
			  {
				if (iResult==0)
				{
				  sprintf_s(error_msg,"Write unexpected close of connection at remote end\n");
				}
				else
				{
				  sprintf_s(error_msg,"Write response was too short - %d chars\n", iResult);
				}
				return false;
			  }
			  else if (ibuf[7] & 0x80)
			  {
				sprintf_s(error_msg,"Write MODBUS exception response - type %d\n", ibuf[8]);
				printf(error_msg);
				return false;
			  }

			  uint16_t addr = ibuf[8] << 8 | ibuf[9];
			  uint16_t num = ibuf[10] << 8  | ibuf[11];

			  if ((addr == reg_no ) && (num == num_regs)) 
				  return true;
			  else
			  {
				  sprintf_s(error_msg,"Write something wrong");
				  return false;
			  }

			  delete [] obuf;
			  delete [] ibuf;
			  return true;

		}


		/* Get a float from 4 bytes in Modbus format */
		float modbus_get_float(const uint16_t *src)
		{
			float f = 0.0f;
			uint32_t i;

			i = (((uint32_t)src[1]) << 16) + src[0];
			memcpy(&f, &i, sizeof(float));

			return f;
		}

		/* Set a float to 4 bytes in Modbus format */
		void modbus_set_float(float f, uint16_t *dest)
		{
			uint32_t i = 0;

			memcpy(&i, &f, sizeof(uint32_t));
			dest[0] = (uint16_t)i;
			dest[1] = (uint16_t)(i >> 16);
		}

};