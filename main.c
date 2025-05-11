#include "hashtable.h"
#include "STL.h"

int main(void)
{

    int row = 0; // Lưu số dòng của file awl
    // 0. Tạo bảng băm đóng trống
    H_InitSaveDataIO();

    // 1.Tạo và đọc địa chỉ file TXT đã xóa comment
    STL_CreatFileNoComment(&row);

    // 2.1  Chuyển về List 1 gồm tập hợp các ký tự có loại bỏ các ký tự ko cần thiết
    STL_CreatList(row);

    // 2.2 Gom các ký tự câu lệnh (biến) vào 1 data ,
    //      bổ sung ! vào biến thường đóng , bổ xung "sl" nếu là suòn lên hoặc "sx" nếu là sườn xuống

    STL_EditList();

    // 2.3 .Lưu các I , Q ,M , T, C vào trong bảng băm
    STL_SaveDataIO();

    // 2.4Tách nhánh
    STL_SplitBranch();
    // 3 .Tao file DataPLC.c với các thành phần phụ trợ
    STL_FileData();

    STL_InsertListToFileData();

    STL_AddTimerFuntion();

    STL_AddReadWriteFunction();

    // 4. Tạo file DataPLC.h
    STL_FileDefineData();

    return 0;
}



