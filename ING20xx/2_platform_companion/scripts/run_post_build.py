import sys
import os
import subprocess
import shutil
import argparse

def main():
    parser = argparse.ArgumentParser(
        description='Run post-build process for platform companion',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python run_post_build.py --address 0x2002000 --app_name app_project --sdk_path D:\\path\\to\\sdk
        '''
    )
    
    parser.add_argument(
        '--address',
        type=str,
        required=True,
        help='Platform address (reserved, not used yet)'
    )
    
    parser.add_argument(
        '--app_name',
        type=str,
        required=True,
        help='App project name'
    )
    
    parser.add_argument(
        '--sdk_path',
        type=str,
        required=True,
        help='SDK path'
    )
    
    args = parser.parse_args()
    
    try:
        address_int = int(args.address, 0)
    except ValueError:
        print('Error: Invalid address format')
        sys.exit(1)
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    platform_companion_dir = os.path.dirname(script_dir)
    base_path = os.path.dirname(platform_companion_dir)
    
    python_script_name = 'make_bundle.py'
    gen_files_tool_name = 'gen_files.nim'
    gen_files_tool_cmd = 'gen_files'
    source_dir = 'generated'
    dest_subdir = 'sdk\\bundles\\rom\\ING20xx'
    
    script_path = script_dir
    source_path = os.path.join(platform_companion_dir, source_dir)
    dest_path = os.path.join(base_path, args.app_name, dest_subdir)
    gen_files_target = os.path.join(base_path, args.app_name, 'sdk', 'bundles')
    
    gen_files_tool_dir = os.path.join(args.sdk_path, 'tools')
    gen_files_tool_path = os.path.join(gen_files_tool_dir, gen_files_tool_name)
    
    print('Current working directory: {}'.format(os.getcwd()))
    print()
    
    print('[Step 1] Executing Python script: {}'.format(python_script_name))
    python_script_path = os.path.join(script_path, python_script_name)
    result = subprocess.run([sys.executable, python_script_path, '--app_addr', args.address], cwd=script_path)
    if result.returncode != 0:
        print('Error: Python script execution failed')
        sys.exit(1)
    print('[Step 1] Python script execution completed')
    print()
    
    print('[Step 2] Copying files to destination directory...')
    if not os.path.exists(source_path):
        print('Error: Source directory not found: {}'.format(source_path))
        sys.exit(1)
    
    if not os.path.exists(dest_path):
        print('Creating destination directory: {}'.format(dest_path))
        os.makedirs(dest_path)
    
    for item in os.listdir(source_path):
        src_item = os.path.join(source_path, item)
        dst_item = os.path.join(dest_path, item)
        if os.path.isdir(src_item):
            shutil.copytree(src_item, dst_item, dirs_exist_ok=True)
        else:
            shutil.copy2(src_item, dst_item)
    print('[Step 2] File copy operation completed')
    print()
    
    if not os.path.exists(gen_files_tool_path):
        print('Error: gen_files tool not found: {}'.format(gen_files_tool_path))
        sys.exit(1)
    
    os.chdir(gen_files_tool_dir)
    print('Current working directory: {}'.format(os.getcwd()))
    print()
    
    print('[Step 3] Executing gen_files tool...')
    result = subprocess.run([gen_files_tool_cmd, gen_files_target], cwd=gen_files_tool_dir)
    if result.returncode != 0:
        print('Error: gen_files execution failed')
        sys.exit(1)
    print('[Step 3] gen_files execution completed')
    print()
    
    print('All steps completed successfully!')

if __name__ == '__main__':
    main()
