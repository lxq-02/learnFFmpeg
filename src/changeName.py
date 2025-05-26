import sys
import os

def change_name():
    if len(sys.argv) != 3:
        print("用法：python script.py 原文件名 新文件夹名")
        sys.exit(1)

    old_folder_name = sys.argv[1]
    new_folder_name = sys.argv[2]

    print(f"参数1是: {old_folder_name}")
    print(f"参数2是: {new_folder_name}")

    # 1、进入文件夹
    current_dir = os.path.dirname(os.path.abspath(__file__))
    target_path = os.path.join(current_dir, old_folder_name)

    if not os.path.exists(target_path):
        print(f"目录不存在：{target_path}")
        sys.exit(1)

    os.chdir(target_path)
    print(f"已进入目录：{os.getcwd()}")

    # 2、更改4个文件名
    old_file_prefix = old_folder_name.replace(" - 副本", "")
    new_file_prefix = new_folder_name
    extensions = ['.sln', '.vcxproj', '.vcxproj.filters', '.vcxproj.user', '.cpp']
    # 2.1 .sln
    # 2.2 .vcxproj
    # 2.3 .vcxproj.filters
    # 2.4 .vcxproj.user
    for ext in extensions:
        old_filename = f"{old_file_prefix}{ext}"
        new_filename = f"{new_file_prefix}{ext}"

        if os.path.exists(old_filename):
            os.rename(old_filename, new_filename)
            print(f"重命名 {old_filename} -> {new_filename}")
        else:
            print(f"未找到文件:{old_filename}")

    # 3、替换.sln中的字符串
    sln_file = f"{new_folder_name}.sln"
    # 读取文件内容
    with open(sln_file, 'r', encoding='utf-8') as f:
        content = f.read()

    old_str = old_file_prefix
    new_str = new_folder_name
    new_content = content.replace(old_str, new_str)

    with open(sln_file, 'w', encoding='utf-8') as f:
        f.write(new_content)

    # 4、回到上一层位置，改文件名
    os.chdir('..')
    print(f"回到上一层目录:{os.getcwd()}")
    if os.path.exists(old_folder_name):
        os.rename(old_folder_name, new_folder_name)
        print(f"重命名文件夹 {old_folder_name} -> {new_folder_name}")
    else:
        print(f"未找到文件夹: {old_folder_name}")

if __name__ == '__main__':
    change_name()
