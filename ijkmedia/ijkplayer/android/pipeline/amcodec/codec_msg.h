/**
* @file codec_msg.h
* @brief  Function prototype of codec error
* @author Zhang Chen <chen.zhang@amlogic.com>
* @version 1.0.0
* @date 2011-02-24
*/
/* Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
* 
*/
#ifndef CODEC_MSG_H
#define CODEC_MSG_H

const char * codec_error_msg(int error);
int system_error_to_codec_error(int error);
void print_error_msg(int error, int syserr, char *func, int line);

#endif
