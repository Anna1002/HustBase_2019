#ifndef RC_HH
#define RC_HH

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum {
	SUCCESS,
	SQL_SYNTAX,
	PF_EXIST,
	PF_FILEERR,
	PF_INVALIDNAME,
	PF_WINDOWS,
	PF_FHCLOSED,
	PF_FHOPEN,
	PF_PHCLOSED,
	PF_PHOPEN,
	PF_NOBUF,
	PF_EOF,
	PF_INVALIDPAGENUM,
	PF_NOTINBUF,
	PF_PAGEPINNED,
	RM_FHCLOSED,
	RM_FHOPENNED,
	RM_INVALIDRECSIZE,
	RM_INVALIDRID,
	RM_FSCLOSED,
	RM_NOMORERECINMEM,
	RM_FSOPEN,
	RM_EOF,
	IX_INVALIDKEYSIZE,
	IX_IHOPENNED,
	IX_IHCLOSED,
	IX_DELETE_NO_RID,
	IX_DELETE_NO_KEY,
	IX_CHILD_NODE_UNDERFLOW,
	IX_CHILD_NODE_OVERFLOW,
	IX_INVALIDKEY,
	IX_NOMEM,
	RM_NOMOREIDXINMEM,
	IX_EOF,
	IX_SCANCLOSED,
	IX_ISCLOSED,
	IX_NOMOREIDXINMEM,
	IX_SCANOPENNED,
	FAIL,
	OS_FAIL,

	DB_EXIST,
	DB_NOT_EXIST,
	NO_DB_OPENED,
	DB_NAME_ILLEGAL,

	TABLE_NOT_EXIST,
	TABLE_EXIST,
	TABLE_NAME_ILLEGAL,

	ATTR_NOT_EXIST,
	ATTR_EXIST,
	TOO_MANY_ATTR,
	INVALID_ATTRS,
	INVALID_TYPES,
	INVALID_VALUES,

	INVALID_CONDITIONS,

	FLIED_NOT_EXIST,//在不存在的字段上增加索引
	FIELD_NAME_ILLEGAL,
	FIELD_MISSING,//插入的时候字段不足
	FIELD_REDUNDAN,//插入的时候字段太多
	FIELD_TYPE_MISMATCH,//字段类型有误

	RECORD_NOT_EXIST,//对一条不存在的记录进行删改时

	INDEX_NAME_REPEAT,
	INDEX_EXIST,//在指定字段上，已经存在索引了
	INDEX_NOT_EXIST,
	INDEX_NAME_ILLEGAL,

	NOT_SELECT,
}RC;

#endif