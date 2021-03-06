#include "message.h"

#include <arpa/inet.h>

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


msg_type get_msg_type (const char *msg)
{
  char type[MSG_TYPE_SIZE + 1];
  strncpy (type, msg, MSG_TYPE_SIZE);
  type[MSG_TYPE_SIZE] = '\0';
  
  if     (strcmp("ACKM", type) == 0) return ACKM;
  else if(strcmp("DIFF", type) == 0) return DIFF;
  else if(strcmp("ENDM", type) == 0) return ENDM;
  else if(strcmp("IMOK", type) == 0) return IMOK;
  else if(strcmp("ITEM", type) == 0) return ITEM;
  else if(strcmp("LAST", type) == 0) return LAST;
  else if(strcmp("LINB", type) == 0) return LINB;
  else if(strcmp("LIST", type) == 0) return LIST;
  else if(strcmp("MESS", type) == 0) return MESS;
  else if(strcmp("OLDM", type) == 0) return OLDM;
  else if(strcmp("REGI", type) == 0) return REGI;
  else if(strcmp("RENO", type) == 0) return RENO;
  else if(strcmp("REOK", type) == 0) return REOK;
  else if(strcmp("RUOK", type) == 0) return RUOK;

  return -1;
}

int msglen (msg_type type)
{
  size_t len = MSG_TYPE_SIZE + MSG_END_SIZE;
  
  switch (type)
    {
    case DIFF:
    case OLDM:
      return len + MSG_NUM_MESS_SIZE + MSG_ID_SIZE + MSG_MESS_SIZE + 3; // 161

    case MESS:
      return len + MSG_ID_SIZE + MSG_MESS_SIZE + 2; // 156
      
    case ITEM:
    case REGI:
      return len + MSG_ID_SIZE + 2*MSG_IP_SIZE + 2*MSG_PORT_SIZE + 5; // 57

    case LAST:
      return len + MSG_NB_MESS_SIZE + 1; // 10

    case LINB:
      return len + MSG_NUM_DIFF_SIZE + 1; // 9

    case ACKM:
    case ENDM:
    case IMOK:
    case LIST:
    case RENO:
    case REOK:
    case RUOK:
      return len; // 6
    }

  return -1;
}

const char* msg_type_to_str (msg_type type)
{
  switch (type)
    {
    case ACKM:
      return "ACKM";
    case DIFF:
      return "DIFF";
    case ENDM:
      return "ENDM";
    case IMOK:
      return "IMOK";
    case ITEM:
      return "ITEM";
    case LAST:
      return "LAST";
    case LINB:
      return "LINB";
    case LIST:
      return "LIST";
    case MESS:
      return "MESS";
    case OLDM:
      return "OLDM";
    case REGI:
      return "REGI";
    case RENO:
      return "RENO";
    case REOK:
      return "REOK";
    case RUOK:
      return "RUOK";
    }

  return NULL;
}

static void set_msg_type (char *buf, msg_type type)
{
  strcpy (buf, msg_type_to_str(type));
}

static void add_msg_end (char *buf)
{
  strcat (buf, MSG_END);
}

static void add_int (char *buf, int nb, int nb_width)
{
  char tmp[nb_width + 1];
  snprintf (tmp, nb_width + 1, "%.*d", nb_width, nb);
  strcat(buf, tmp);
}

static void add_separator (char *buf)
{
  size_t buf_len = strlen(buf);
  buf[buf_len] = SEPARATOR_CHAR;
  buf[buf_len + 1] = '\0';
}

static void add_string (char *buf, const char *str, size_t total_width)
{
  size_t buf_len = strlen(buf);
  char *buf_end = buf + buf_len;  
  char *end = stpncpy (buf_end, str, total_width); // si ne compile pas : utiliser snprintf...
  memset (end, DEFAULT_CHAR, total_width - (end - buf_end));
  buf_end[total_width] = '\0';
}

static void add_ip_addr (char *buf, struct in_addr *inp)
{
  char tmp[INET_ADDRSTRLEN];
  size_t buf_len = strlen(buf);
  char *buf_end = buf + buf_len;  

  inet_ntop(AF_INET, inp, tmp, INET_ADDRSTRLEN);  
  memset (buf_end, DEFAULT_INT, MSG_IP_SIZE);
  char *tok = strtok(tmp, ".");
  for (int i=0; tok; tok = strtok(NULL, "."))
    {
      strcpy(4*i + buf_end + 3 - strlen(tok), tok);
      i++;
    }
  buf_end[3] = '.';
  buf_end[7] = '.';
  buf_end[11] = '.';
}

char* create_message (char* buf, msg_type type, ...)
{
  va_list args;
  char *ret;
  int len;

  
  len = msglen (type);
  if (len < 0)
    return NULL;

  
  if (buf == NULL)
    {
      ret = malloc (len + 1); // +1 pour le '\0' de fin
      assert (ret);
    }
  else
    {
      ret = buf;
    }

  va_start (args, type);

  
  set_msg_type (ret, type);  

  switch (type)
    {
    case DIFF:      
    case OLDM:
      add_separator (ret);
      add_int (ret, va_arg(args, int), MSG_NUM_MESS_SIZE);
      add_separator (ret);
      add_string (ret, va_arg(args, char*), MSG_ID_SIZE);
      add_separator (ret);
      add_string (ret, va_arg(args, char*), MSG_MESS_SIZE);
      break;
      
    case MESS:
      add_separator (ret);
      add_string (ret, va_arg(args, char*), MSG_ID_SIZE);
      add_separator (ret);
      add_string (ret, va_arg(args, char*), MSG_MESS_SIZE);
      break;
      
    case ITEM:
    case REGI:
      add_separator (ret);
      add_string (ret, va_arg(args, char*), MSG_ID_SIZE);
      add_separator (ret);
      add_ip_addr(ret, va_arg(args, struct in_addr*));
      add_separator (ret);
      add_int (ret, va_arg(args, int), MSG_NUM_MESS_SIZE);
      add_separator (ret);
      add_ip_addr(ret, va_arg(args, struct in_addr*));
      add_separator (ret);
      add_int (ret, va_arg(args, int), MSG_NUM_MESS_SIZE);
      break;
      
    case LAST:
      add_separator (ret);
      add_int (ret, va_arg(args, int), MSG_NB_MESS_SIZE);
      break;
      
    case LINB:      
      add_separator (ret);
      add_int (ret, va_arg(args, int), MSG_NUM_DIFF_SIZE);
      break;
            
    default: //ACKM, ENDM, IMOK, LIST, RENO, REOK, RUOK
      break;
    } 

  
  va_end (args);

  add_msg_end (ret);
  
  return ret;
}

static bool verify_int (const char *msg, int width)
{
  if (!msg)
    return false;
  
  for (int i=0; i < width; i++)
    {
      if (!isdigit(msg[i]))
	return false;
    }
  
  return true;
}

static bool verify_separator (const char *msg)
{
  return msg && msg[0] == SEPARATOR_CHAR;
}

static bool verify_end (const char *msg)
{
  return msg && msg[0] == MSG_END[0] && msg[1] == MSG_END[1];
}

static bool verify_ip_addr (const char *msg)
{
  return msg
    && verify_int(msg, 3)
    && msg[3] == '.'
    && verify_int(msg + 4, 3)
    && msg[7] == '.'
    && verify_int(msg + 8, 3)
    && msg[11] == '.'
    && verify_int(msg + 12, 3);  
}

bool verify_msg (const char *msg)
{
  msg_type type = get_msg_type (msg);
  size_t len = strlen(msg);
  if (type < 0 || len < 6 || len > MAX_MSG_SIZE)
    return false;    
    
  switch (type)
    {
    case DIFF:
    case OLDM:
      return len == msglen(DIFF)
	&& verify_separator (msg + MSG_TYPE_SIZE)
	&& verify_int (msg + MSG_TYPE_SIZE + 1, MSG_NUM_MESS_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 1 + MSG_NUM_MESS_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 2 + MSG_NUM_MESS_SIZE + MSG_MESS_SIZE)
	&& verify_end (msg + MSG_TYPE_SIZE + MSG_NUM_MESS_SIZE + MSG_ID_SIZE + MSG_MESS_SIZE + 3);
	
    case MESS:
      return len == msglen(MESS)
	&& verify_separator (msg + MSG_TYPE_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 1 + MSG_ID_SIZE)
	&& verify_end (msg + MSG_TYPE_SIZE + MSG_ID_SIZE + MSG_MESS_SIZE + 2);
      
    case ITEM:
    case REGI:
      return len == msglen(ITEM)
	&& verify_separator (msg + MSG_TYPE_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 1 + MSG_ID_SIZE)
	&& verify_ip_addr   (msg + MSG_TYPE_SIZE + 2 + MSG_ID_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 2 + MSG_ID_SIZE + MSG_IP_SIZE)
	&& verify_int       (msg + MSG_TYPE_SIZE + 3 + MSG_ID_SIZE + MSG_IP_SIZE, MSG_PORT_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 3 + MSG_ID_SIZE + MSG_IP_SIZE + MSG_PORT_SIZE)
	&& verify_ip_addr   (msg + MSG_TYPE_SIZE + 4 + MSG_ID_SIZE + MSG_IP_SIZE + MSG_PORT_SIZE)
	&& verify_separator (msg + MSG_TYPE_SIZE + 4 + MSG_ID_SIZE + 2*MSG_IP_SIZE + MSG_PORT_SIZE)
	&& verify_int       (msg + MSG_TYPE_SIZE + 5 + MSG_ID_SIZE + 2*MSG_IP_SIZE + MSG_PORT_SIZE, MSG_PORT_SIZE)
	&& verify_end       (msg + MSG_TYPE_SIZE + 5 + MSG_ID_SIZE + 2*MSG_IP_SIZE + 2*MSG_PORT_SIZE);

    case LAST:
      return len == msglen(LAST)
	&& verify_separator (msg + MSG_TYPE_SIZE)
	&& verify_int (msg + MSG_TYPE_SIZE + 1, MSG_NB_MESS_SIZE)
	&& verify_end (msg + MSG_TYPE_SIZE + 1 + MSG_NB_MESS_SIZE);

    case LINB:      
      return len == msglen(LINB)
	&& verify_separator (msg + MSG_TYPE_SIZE)
	&& verify_int (msg + MSG_TYPE_SIZE + 1, MSG_NUM_DIFF_SIZE)
	&& verify_end (msg + MSG_TYPE_SIZE + 1 + MSG_NUM_DIFF_SIZE);
      
      
    case ACKM:
    case ENDM:
    case IMOK:
    case LIST:
    case RENO:
    case REOK:
    case RUOK:      
      return len == msglen(ACKM)
	&& verify_end (msg + MSG_TYPE_SIZE);
    }

  return false;
}
