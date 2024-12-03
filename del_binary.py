import os
import shutil

def delete_binary_folders():
    current_directory = os.getcwd()  # 获取当前文件夹路径
    
    # 使用 os.walk 递归遍历所有子文件夹
    for root, dirs, files in os.walk(current_directory, topdown=False):  # topdown=False 表示从子文件夹开始遍历
        for dir_name in dirs:
            if dir_name == "Binaries" or dir_name == "Intermediate" or dir_name == "Saved":
                folder_path = os.path.join(root, dir_name)
                try:
                    shutil.rmtree(folder_path)  # 删除整个文件夹及其内容
                    print(f"Deleted folder: {folder_path}")
                except Exception as e:
                    print(f"Error deleting folder {folder_path}: {e}")

if __name__ == "__main__":
    delete_binary_folders()
