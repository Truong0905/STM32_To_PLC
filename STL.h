#ifndef __STL_H__
#define __STL_H__

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void STL_CreatFileNoComment(int *RowOfFile); // Tạo và đọc địa chỉ file TXT đã xóa comment
void STL_CreatList(int RowOfFile); // Chuyển về List 1 gồm tập hợp các ký tự có loại bỏ các ký tự ko cần thiết
void STL_EditList(void);           // Gom các ký tự câu lệnh (biến) vào 1 data , bổ sung H vào biến thường đóng , bổ xung "sl" nếu là suòn lên hoặc "sx" nếu là sườn xuống
void STL_SaveDataIO(void);
void STL_SplitBranch(void); // Tách nhánh
void STL_InsertListToFileData(void);
void STL_FileDefineData(void);
void STL_FileData(void);
void STL_AddTimerFuntion(void);
void STL_AddReadWriteFunction(void);

#endif /*__STL_H__*/