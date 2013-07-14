// CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>

// Fuse
#define FUSE_USE_VERSION 29
#include <fuse.h>

// Linux 
#include <errno.h>
#include <fcntl.h>

// stdlib 
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CUDA_API_CALL(ret) if (CUDA_SUCCESS != ret) { fprintf(context->log_fp, "cuda error : %d", ret); exit(-1); }

#define LOG(...) {                                                                        \
  struct afs_context *context = (struct afs_context*)(fuse_get_context()->private_data);  \
  fprintf(context->log_fp, __VA_ARGS__);                                                  \
  fflush(context->log_fp);                                                                \
  }

static const char *afs_str = "Hello World!\n";
static const char *afs_path = "/hello";

typedef unsigned char byte_t;

struct afs_context {
  FILE *log_fp;
  size_t heap_size;
  byte_t *heap;
};

static void *afs_init(struct fuse_conn_info *conn)
{
  struct afs_context *context = (struct afs_context *)malloc(sizeof(struct afs_context));
  context->log_fp = fopen("/home/iitaku/log", "a");
  context->heap_size = strlen(afs_str);
  CUDA_API_CALL(cudaMalloc((void**)&context->heap, context->heap_size));
  CUDA_API_CALL(cudaMemcpy((void*)context->heap, afs_str, context->heap_size, cudaMemcpyHostToDevice));
 
  return (void*)context;
}

void afs_destroy(void * private_data)
{
  LOG("destroy\n");

  struct afs_context *context = (struct afs_context*)private_data;
  CUDA_API_CALL(cudaFree((void*)context->heap));
  fclose(context->log_fp);
  free(private_data);
}

static int afs_getattr(const char *path, struct stat *stbuf)
{
  LOG("getattr\n");
 
  int res = 0;

  memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else if (strcmp(path, afs_path) == 0) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(afs_str);
  } else
    res = -ENOENT;

  return res;
}

static int afs_getxattr(const char *path, const char *name, char *value, size_t size)
{
  LOG("getxattr\n");

  struct afs_context *context = (struct afs_context*)(fuse_get_context()->private_data);
  
  cudaIpcMemHandle_t handle;
  CUDA_API_CALL(cudaIpcGetMemHandle(&handle, context->heap));

  return size;
}

static int afs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fi)
{
  LOG("readdir\n");
  
  (void) offset;
  (void) fi;

  if (strcmp(path, "/") != 0)
    return -ENOENT;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  filler(buf, afs_path + 1, NULL, 0);

  return 0;
}

static int afs_open(const char *path, struct fuse_file_info *fi)
{
  LOG("open\n");
  
  if (strcmp(path, afs_path) != 0)
    return -ENOENT;

  if ((fi->flags & 3) != O_RDONLY)
    return -EACCES;

  return 0;
}

static int afs_read(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi)
{
  LOG("read\n");
  
  size_t len;
  (void) fi;
  if(strcmp(path, afs_path) != 0)
    return -ENOENT;

  len = strlen(afs_str);
  if (offset < len) {
    if (offset + size > len)
      size = len - offset;
    struct afs_context *context = (struct afs_context*)(fuse_get_context()->private_data);
    CUDA_API_CALL(cudaMemcpy(buf, (void*)(context->heap+offset), size, cudaMemcpyDeviceToHost));
  } else
    size = 0;

  return size;
}

static struct fuse_operations afs_oper = {
  .init       = afs_init,
  .getattr    = afs_getattr,
  .getxattr   = afs_getxattr,
  .readdir    = afs_readdir,
  .open       = afs_open,
  .read       = afs_read,
};

int main(int argc, char *argv[])
{
  return fuse_main(argc, argv, &afs_oper, NULL);
}

