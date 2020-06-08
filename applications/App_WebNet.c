/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-17     CCRFIDZY       the first version
 */
/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-16     CCRFIDZY       the first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <webnet.h>
#include <wn_module.h>
#include "App_Webnet.h"
#include "APP_Core.h"

#include "drv_flash.h"
#include <rtthread.h>
#include <dfs_posix.h>

/**
 * upload file.
 */
static const char * sd_upload = "/webnet";
static const char * upload_dir = "upload"; /* e.g: "upload" */
const struct fal_partition *fal_part;
static int file_size = 0;
	
#define SYSTEMBIN  "APP.bin"
#define BLEBIN     "BLE.bin"
#define UWBBIN     "UWB.bin"
#define FACBIN     "factory.bin"

static void showout(struct webnet_session* session)
{
		uint8_t BUFF[100];
    const char* mimetype;
    struct webnet_request* request;
    static const char* header = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; "
                                "charset=gbk\" /><title> Equipment state </title></head>";
		
    static const char* body = "<body><form method=\"post\" action=\"/cgi-bin/showout\">"
                              "<h3>Net Setting</h3>"
															"<h3>DHCP(1(ON) or 0(OFF))   <input type=\"text\" style=\"height:20px;width:20px\" name=\"DHCP\" value = \"%d\"></h3><hr>"
                              "<h3>Client IP   "
                              "<input type=\"text\" style=\"height:20px;width:30px\" name=\"CIP1\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"CIP2\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"CIP3\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"CIP4\" value = \"%d\"></h3><hr>"
                              "<h3>Client PORT   "
                              "<input type=\"text\" style=\"height:20px;width:50px\" name=\"CPort\" value = \"%d\"></h3><hr>"
                              "<h3>Work Mode(1(TCP) or 2(UDP))   "
                              "<input type=\"text\" style=\"height:20px;width:20px\" name=\"WorkMode\" value = \"%d\"></h3><hr>"
                              "<h3>Mask   "
                              "<input type=\"text\" style=\"height:20px;width:30px\" name=\"Mask1\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Mask2\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Mask3\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Mask4\" value = \"%d\"></h3><hr>"
                              "<h3>Gateway   "
                              "<input type=\"text\" style=\"height:20px;width:30px\" name=\"Gateway1\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Gateway2\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Gateway3\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"Gateway4\" value = \"%d\"></h3><hr>"
															"<h3>Serve IP   "
                              "<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP1\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP2\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP3\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP4\" value = \"%d\"></h3><hr>"
                              "<h3>Serve PORT   "
                              "<input type=\"text\" style=\"height:20px;width:50px\" name=\"SPort\" value = \"%d\"></h3><hr>"
															"<h3>Serve IP2   "
                              "<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP21\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP22\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP23\" value = \"%d\">."
															"<input type=\"text\" style=\"height:20px;width:30px\" name=\"SIP24\" value = \"%d\"></h3><hr>"
                              "<h3>Serve PORT2   "
                              "<input type=\"text\" style=\"height:20px;width:50px\" name=\"SPort2\" value = \"%d\"></h3><hr>"					
															"<input type=\"submit\" value=\"Set\"></form><hr><br>"
                              "<br><a href=\"/index.html\">Go back to root</a></body></html>\r\n";
	static const char* body2 =  "<body><form method=\"post\" action=\"/cgi-bin/showout\">"
                              "<h3>Set Success!</h3>"
                              "<br><a href=\"/index.html\">Go back to root</a></body></html>\r\n";

    RT_ASSERT(session != RT_NULL);
    request = session->request;
    RT_ASSERT(request != RT_NULL);

    /* get mimetype */
    mimetype = mime_get_type(".html");
		
    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", -1);
    webnet_session_write(session, (const rt_uint8_t*)header, rt_strlen(header));
    stm32_flash_read(Memery_Start_Address,BUFF,100);
		if (request->query_counter)
    {
			webnet_session_printf(session, body2);
			const char *DHCP,*CIP1,*CIP2,*CIP3,*CIP4,*CPort,*WorkMode,*Mask1,*Mask2,*Mask3,*Mask4, \
				*Gateway1,*Gateway2,*Gateway3,*Gateway4,*SIP1,*SIP2,*SIP3,*SIP4,*SPort,*SIP21,*SIP22,*SIP23,*SIP24,*SPort2;
			DHCP = webnet_request_get_query(request, "DHCP");
			CIP1 = webnet_request_get_query(request, "CIP1");
			CIP2 = webnet_request_get_query(request, "CIP2");
			CIP3 = webnet_request_get_query(request, "CIP3");
			CIP4 = webnet_request_get_query(request, "CIP4");
			CPort = webnet_request_get_query(request, "CPort");
			WorkMode = webnet_request_get_query(request, "WorkMode");
			Mask1 = webnet_request_get_query(request, "Mask1");
			Mask2 = webnet_request_get_query(request, "Mask2");
			Mask3 = webnet_request_get_query(request, "Mask3");
			Mask4 = webnet_request_get_query(request, "Mask4");
			Gateway1 = webnet_request_get_query(request, "Gateway1");
			Gateway2 = webnet_request_get_query(request, "Gateway2");
			Gateway3 = webnet_request_get_query(request, "Gateway3");
			Gateway4 = webnet_request_get_query(request, "Gateway4");
			SIP1 = webnet_request_get_query(request, "SIP1");
			SIP2 = webnet_request_get_query(request, "SIP2");
			SIP3 = webnet_request_get_query(request, "SIP3");
			SIP4 = webnet_request_get_query(request, "SIP4");
			SPort = webnet_request_get_query(request, "SPort");
			SIP21 = webnet_request_get_query(request, "SIP21");
			SIP22 = webnet_request_get_query(request, "SIP22");
			SIP23 = webnet_request_get_query(request, "SIP23");
			SIP24 = webnet_request_get_query(request, "SIP24");
			SPort2 = webnet_request_get_query(request, "SPort2");
			BUFF[DHCP_Add] = atoi(DHCP);
			BUFF[CIP1_Add] = atoi(CIP1);
			BUFF[CIP2_Add] = atoi(CIP2);
			BUFF[CIP3_Add] = atoi(CIP3);
			BUFF[CIP4_Add] = atoi(CIP4);
			BUFF[CPort1_Add] = atoi(CPort)>>8;
			BUFF[CPort2_Add] = atoi(CPort);
			BUFF[WorkMode_Add] = atoi(WorkMode);
			BUFF[Mask1_Add] = atoi(Mask1);
			BUFF[Mask2_Add] = atoi(Mask2);
			BUFF[Mask3_Add] = atoi(Mask3);
			BUFF[Mask4_Add] = atoi(Mask4);
			BUFF[Gateway1_Add] = atoi(Gateway1);
			BUFF[Gateway2_Add] = atoi(Gateway2);
			BUFF[Gateway3_Add] = atoi(Gateway3);
			BUFF[Gateway4_Add] = atoi(Gateway4);
			BUFF[SIP1_Add] = atoi(SIP1);
			BUFF[SIP2_Add] = atoi(SIP2);
			BUFF[SIP3_Add] = atoi(SIP3);
			BUFF[SIP4_Add] = atoi(SIP4);
			BUFF[SPort1_Add] = atoi(SPort)>>8;
			BUFF[SPort2_Add] = atoi(SPort);
			BUFF[SIPx11_Add] = atoi(SIP21);
			BUFF[SIPx12_Add] = atoi(SIP22);
			BUFF[SIPx13_Add] = atoi(SIP23);
			BUFF[SIPx14_Add] = atoi(SIP24);
			BUFF[SPortx11_Add] = atoi(SPort2)>>8;
			BUFF[SPortx12_Add] = atoi(SPort2);
			
			stm32_flash_erase(Memery_Start_Address,1);
			stm32_flash_write(Memery_Start_Address,BUFF,100);
			rt_event_send(&Core_EVENT,EVENT_REBOOT);
    }
		else
		{
			uint16_t cport,sport,sport2;
			cport = BUFF[CPort1_Add];
			cport = (cport<<8) + BUFF[CPort2_Add];
			sport = BUFF[SPort1_Add];
			sport = (sport<<8) + BUFF[SPort2_Add];
			sport2 = BUFF[SPortx11_Add];
			sport2 = (sport2<<8) + BUFF[SPortx12_Add];
			webnet_session_printf(session, body,BUFF[DHCP_Add],BUFF[CIP1_Add],BUFF[CIP2_Add],BUFF[CIP3_Add],BUFF[CIP4_Add],cport,BUFF[WorkMode_Add],	\
																					BUFF[Mask1_Add],BUFF[Mask2_Add],BUFF[Mask3_Add],BUFF[Mask4_Add],BUFF[Gateway1_Add],BUFF[Gateway2_Add],	\
																					BUFF[Gateway3_Add],BUFF[Gateway4_Add],BUFF[SIP1_Add],BUFF[SIP2_Add],BUFF[SIP3_Add],BUFF[SIP4_Add],sport, \
																					BUFF[SIPx11_Add],BUFF[SIPx12_Add],BUFF[SIPx13_Add],BUFF[SIPx14_Add],sport2);
		}
}

static void bootloader(struct webnet_session* session)
{
	uint8_t buf[100];
	char CCRFID[6];
	const char* mimetype;
	struct webnet_request* request;
	static const char* header = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; "
																 "charset=gbk\" /><title> bootloader </title></head>";

	static const char* body = "<body><form method=\"post\" action=\"/cgi-bin/bootloader\">"
															 "<h3>%s</h3>"
															 "<br><a href=\"/index.html\">Go back</a></body></html>\r\n";

	RT_ASSERT(session != RT_NULL);
	request = session->request;
	RT_ASSERT(request != RT_NULL);

	/* get mimetype */
	mimetype = mime_get_type(".html");

	/* set http header */
	session->request->result_code = 200;
	webnet_session_set_header(session, mimetype, 200, "Ok", -1);

	webnet_session_write(session, (const rt_uint8_t*)header, rt_strlen(header));
	stm32_flash_read(Memery_Start_Address,buf,100);
	//bin文件长度不为0
	if((buf[BOOT_SIZE] == 0) && (buf[BOOT_SIZE+1] == 0)&& (buf[BOOT_SIZE+2] == 0)&& (buf[BOOT_SIZE+3] == 0))
	{
		webnet_session_printf(session, body,"No File!");
	}
	else
	{
		fal_part = fal_partition_find("systembin");
		fal_partition_read(fal_part,0x4000,(uint8_t *)CCRFID,7);
		if(strcmp((char *)CCRFID,"CCRFID")==0)
		{
			webnet_session_printf(session, body,"Download Success!");
			stm32_flash_read(Memery_Start_Address,buf,100);
			stm32_flash_erase(Memery_Start_Address,1);
			buf[BOOT_FLAG] = 0x01;
			stm32_flash_write(Memery_Start_Address,buf,100);
			HAL_NVIC_SystemReset();
		}
		else
			webnet_session_printf(session, body,"Check digital sign Fail!");
	}
}

static void BLEupload(struct webnet_session* session)
{
		uint8_t blesize[4];
		uint64_t blesize_64=0;
		const char* mimetype;
		struct webnet_request* request;
		static const char* header = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; "
																 "charset=gbk\" /><title> BLEupload </title></head>";

		static const char* body = "<body><form method=\"post\" action=\"/cgi-bin/BLEupload\">"
															 "<h3>%s</h3>"
															 "<br><a href=\"/index.html\">Go back</a></body></html>\r\n";
		RT_ASSERT(session != RT_NULL);
		request = session->request;
		RT_ASSERT(request != RT_NULL);

		/* get mimetype */
		mimetype = mime_get_type(".html");

		/* set http header */
		session->request->result_code = 200;
		
		webnet_session_set_header(session, mimetype, 200, "Ok", -1);
		
		webnet_session_write(session, (const rt_uint8_t*)header, rt_strlen(header));
		
		stm32_flash_read(Memery_Start_Address+BLE_SIZE,blesize,4);
		blesize_64 = (uint64_t)blesize[0];
		blesize_64 += ((uint64_t)blesize[1])<<8;
		blesize_64 += ((uint64_t)blesize[2])<<16;
		blesize_64 += ((uint64_t)blesize[3])<<24;
		if(blesize_64 == 0)
		{
			webnet_session_printf(session, body,"No File!");
		}
		else
		{
			webnet_session_printf(session, body,"Loading...");
		}
}

static void UWBupload(struct webnet_session* session)
{
		uint8_t blesize[4];
		uint64_t blesize_64=0;
		const char* mimetype;
		struct webnet_request* request;
		static const char* header = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; "
																 "charset=gbk\" /><title> UWBupload </title></head>";

		static const char* body = "<body><form method=\"post\" action=\"/cgi-bin/UWBupload\">"
															 "<h3>%s</h3>"
															 "<br><a href=\"/index.html\">Go back</a></body></html>\r\n";
		RT_ASSERT(session != RT_NULL);
		request = session->request;
		RT_ASSERT(request != RT_NULL);

		/* get mimetype */
		mimetype = mime_get_type(".html");

		/* set http header */
		session->request->result_code = 200;
		
		webnet_session_set_header(session, mimetype, 200, "Ok", -1);
		
		webnet_session_write(session, (const rt_uint8_t*)header, rt_strlen(header));
		
		stm32_flash_read(Memery_Start_Address+UWB_SIZE,blesize,4);
		blesize_64 = (uint64_t)blesize[0];
		blesize_64 += ((uint64_t)blesize[1])<<8;
		blesize_64 += ((uint64_t)blesize[2])<<16;
		blesize_64 += ((uint64_t)blesize[3])<<24;
		if((blesize_64 == 0)||(blesize_64 == 0xFFFFFFFF))
		{
			webnet_session_printf(session, body,"No File!");
		}
		else
		{				
			uint8_t databuf[1024];
			fal_part = fal_partition_find("uwbbin");

			target_flash_init(UWB_SWD,0x08000000);	
			//擦除
			for(uint32_t addr = 0; addr < blesize_64; addr += 1024)
			{
				if (0 != target_flash_erase_sector(UWB_SWD,0x08000000+addr))
					break;
			}
			//烧入
			for(uint32_t addr = 0; addr < blesize_64; addr += 1024)
			{
				fal_partition_read(fal_part,addr,databuf,1024);
				target_flash_program_page(UWB_SWD,0x08000000+addr, databuf, 1024);
			}
			//运行
			target_flash_uninit(UWB_SWD);
			webnet_session_printf(session, body,"SUCCESS !!!");
			
			rt_event_send(&Core_EVENT,EVENT_REBOOT);
		}
	}

void WebNet_Start(void)
{
#ifdef WEBNET_USING_CGI
    webnet_cgi_register("bootloader", bootloader);
    webnet_cgi_register("BLEupload", BLEupload);
    webnet_cgi_register("showout", showout);
		webnet_cgi_register("UWBupload", UWBupload);
#endif

#ifdef WEBNET_USING_UPLOAD
    extern const struct webnet_module_upload_entry upload_entry_upload;
    webnet_upload_add(&upload_entry_upload);
#endif
    webnet_init();
}

const char *get_file_name(struct webnet_session *session)
{
    const char *path = RT_NULL, *path_last = RT_NULL;

    path_last = webnet_upload_get_filename(session);
    if (path_last == RT_NULL)
    {
        rt_kprintf("file name err!!\n");
        return RT_NULL;
    }

    path = strrchr(path_last, '\\');
    if (path != RT_NULL)
    {
        path++;
        path_last = path;
    }

    path = strrchr(path_last, '/');
    if (path != RT_NULL)
    {
        path++;
        path_last = path;
    }

    return path_last;
}

static int upload_open(struct webnet_session *session)
{
    int fd;
    const char *file_name = RT_NULL;
    file_name = get_file_name(session);
    rt_kprintf("Upload FileName: %s\n", file_name);

    if (webnet_upload_get_filename(session) != RT_NULL)
    {
        int path_size;
        char * file_path;

        path_size = strlen(sd_upload) + strlen(upload_dir)
                    + strlen(file_name);

        path_size += 4;
        file_path = (char *)rt_malloc(path_size);

        if(file_path == RT_NULL)
        {
            fd = -1;
            goto _exit;
        }

        sprintf(file_path, "%s/%s/%s", sd_upload, upload_dir, file_name);

        rt_kprintf("save to: %s\r\n", file_path);

        fd = open(file_path, O_WRONLY | O_CREAT, 0);
        if (fd < 0)
        {
            webnet_session_close(session);
            rt_free(file_path);

            fd = -1;
            goto _exit;
        }
        else
        {
           //写入分区选择
           if(!strcmp(file_name,SYSTEMBIN))
             fal_part = fal_partition_find("systembin");
           else if(!strcmp(file_name,BLEBIN))
              fal_part = fal_partition_find("blebin");
					 else if(!strcmp(file_name,FACBIN))
							fal_part = fal_partition_find("factory");
					 else if(!strcmp(file_name,UWBBIN))
					 {
							fal_part = fal_partition_find("uwbbin");
							rt_pin_irq_enable(UWB_EXIT_PIN, PIN_IRQ_DISABLE);//关闭SPI上报
					 }
           fal_partition_erase(fal_part,0,fal_part->len);
        }
    }

    file_size = 0;

_exit:
    return (int)fd;
}

static int upload_close(struct webnet_session* session)
{
    int fd;
    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) 
			return 0;
    close(fd);
    rt_kprintf("Upload FileSize: %d\n", file_size);
		rt_pin_irq_enable(UWB_EXIT_PIN, PIN_IRQ_ENABLE);//开启SPI上报
    return 0;
}

static int upload_write(struct webnet_session* session, const void* data, rt_size_t length)
{
    int fd;
    fd = (int)webnet_upload_get_userdata(session);
    if (fd < 0) return 0;

    rt_kprintf("write: length %d\n", length);
    write(fd, data, length);	
		fal_partition_write(fal_part,file_size,data,length);
		file_size += length;
    return length;
}

static int upload_done (struct webnet_session* session)
{
    const char* mimetype;
		uint8_t buf[100];
    static const char* status = "<html><head><title>Upload OK </title>"
                                "</head><body>Upload OK, file length = %d "
                                "<br/><br/><a href=\"javascript:history.go(-1);\">"
                                "Go back to root</a></body></html>\r\n";

    /* get mimetype */
    mimetype = mime_get_type(".html");

    /* set http header */
    session->request->result_code = 200;
    webnet_session_set_header(session, mimetype, 200, "Ok", rt_strlen(status));
    webnet_session_printf(session, status, file_size);
		
		stm32_flash_read(Memery_Start_Address,buf,100);
    stm32_flash_erase(Memery_Start_Address,1);

		if(!rt_strcmp(fal_part->name, "systembin"))
		{
			file_size = file_size - 8;
			buf[BOOT_SIZE] = file_size;
			buf[BOOT_SIZE+1] = file_size>>8;
			buf[BOOT_SIZE+2] = file_size>>16;
			buf[BOOT_SIZE+3] = file_size>>24;
			stm32_flash_write(Memery_Start_Address,buf,100);
		}
		else if(!rt_strcmp(fal_part->name, "blebin"))
		{
			file_size = file_size - 8;
			buf[BLE_SIZE] = file_size;
			buf[BLE_SIZE+1] = file_size>>8;
			buf[BLE_SIZE+2] = file_size>>16;
			buf[BLE_SIZE+3] = file_size>>24;
			stm32_flash_write(Memery_Start_Address,buf,100);
		}
		else if(!rt_strcmp(fal_part->name, "uwbbin"))
		{
			file_size = file_size - 8;
			buf[UWB_SIZE] = file_size;
			buf[UWB_SIZE+1] = file_size>>8;
			buf[UWB_SIZE+2] = file_size>>16;
			buf[UWB_SIZE+3] = file_size>>24;
			stm32_flash_write(Memery_Start_Address,buf,100);
		}
    return 0;
}

const struct webnet_module_upload_entry upload_entry_upload =
{
    "/upload",
    upload_open,
    upload_close,
    upload_write,
    upload_done
};

