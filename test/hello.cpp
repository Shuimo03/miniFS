#define FUSE_USE_VERSION 31

#include<fuse3/fuse.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>


/*
 * 创建文件夹
 * 读取文件夹
 * 展示文件
 * 写入文件
 * 读取文件
 */

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello.txt";

static int hello_getattr(const char *filePath, struct stat *stbuf, fuse_file_info  *fileInfo){
    (void) fileInfo;
    int res =  0;
    memset(stbuf,0,sizeof(struct stat));
    if(strcmp(filePath, "/")==0){
        stbuf->st_mode = S_IFDIR | 0775;
          stbuf->st_nlink = 2;
    }else if(strcmp(filePath,hello_path) == 0){
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1; 
        stbuf->st_size = strlen(hello_str); 
    }else{
        res -= ENOENT;
    }
    return res;
}

static int hello_readdir(const char *filePath,void *buf, fuse_fill_dir_t filler,off_t offset, fuse_file_info *fi, enum fuse_readdir_flags flags){
    (void) offset;
    (void) fi;
    (void) flags;

    if(strcmp(filePath,"/") != 0){
                return -ENOENT;
    }
    filler(buf,".",NULL,0,static_cast<enum fuse_fill_dir_flags>(0));
    filler(buf, "..", NULL, 0, static_cast<enum fuse_fill_dir_flags>(0));
    filler(buf, hello_path + 1, NULL, 0, static_cast<enum fuse_fill_dir_flags>(0));
    
    return 0;

}

static int hello_open(const char *path, fuse_file_info *fi) {
    if (strcmp(path, hello_path) != 0) {
        return -ENOENT;
    }

    if ((fi->flags & O_ACCMODE) != O_RDONLY) {
        return -EACCES;
    }

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, fuse_file_info *fi) {
    (void) fi;

    size_t len = strlen(hello_str);
    if (strcmp(path, hello_path) != 0) {
        return -ENOENT;
    }

    if (offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        memcpy(buf, hello_str + offset, size);
    } else {
        size = 0;
    }

    return size;
}

static int hello_mkdir(const char *path, mode_t mode) {
    int res = mkdir(path, mode);

    if (res == -1 && errno == ENOENT) {
        // 如果父目录不存在，递归创建
        char *dup_path = strdup(path);
        char *last_slash = strrchr(dup_path, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';  // 去掉最后一个斜杠
            res = hello_mkdir(dup_path, 0755);  // 递归创建父目录
            free(dup_path);
            if (res == 0) {
                // 再次尝试创建目标目录
                res = mkdir(path, mode);
            }
        } else {
            // 没有斜杠，直接尝试创建目录
            res = mkdir(path, mode);
        }
    }

    if (res == -1) {
        perror("Failed Create directory");
    }

    return res;
}


static struct fuse_operations hello_oper = {
    .getattr = hello_getattr,
    .readdir = hello_readdir,
    .open = hello_open,
    .read = hello_read,
    .mkdir = hello_mkdir,
};
int main(int argc, char *argv[]) {
    fuse_main(argc, argv, &hello_oper, nullptr);
    return 0;
}