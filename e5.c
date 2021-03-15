#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/gfp.h>
#include <linux/module.h>
#include<linux/kernel.h>
#include<linux/moduleparam.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/slab.h>

struct myfile{
	struct file * f;
	mm_segment_t fs; 
	loff_t pos;
};
struct myfile* open_file_for_read(char* filename)
{
	struct myfile * ff;
	mm_segment_t past_fs;
	int wrong=0;
	ff=(struct myfile*)kmalloc(sizeof(struct myfile), GFP_KERNEL| GFP_NOWAIT);
	past_fs=get_fs();
	ff->fs=get_fs();
	set_fs(get_ds());
	ff->f=filp_open(filename,O_RDONLY,0);
	set_fs(past_fs);
	if(IS_ERR(ff->f)){
		wrong=PTR_ERR(ff->f);
		return NULL;
	}
	return ff;
}
volatile int read_from_file_until (struct myfile* mf, char* buf, unsigned long vlen, char c)
{
	int back=0;
	int iter=0;
	mf->fs=get_fs();
	set_fs(get_ds());

	while (iter != vlen){
		back= vfs_read(mf->f, buf+iter, 1, &(mf->pos));
		if((buf[iter]==c)||buf[iter]=='\0') {break;}
		iter++;
	}
	iter++;
	buf[iter]='\0';
	set_fs(mf->fs);
	return back;
}

void close_file(struct myfile *mf)
{
	filp_close(mf->f, NULL);
}
int init_module (void)
{
	void ** sct;
	int a=0;
	char* pa;
	struct myfile* vis;
	char* t;
	char* ol=kmalloc(sizeof(char)*100, GFP_KERNEL);
	vis = open_file_for_read("/boot/System.map-4.19.0-13-amd64");
	t=kmalloc(sizeof(char)*100, GFP_KERNEL);
	pa=kmalloc(sizeof(char)*100, GFP_KERNEL);
	t="sys_call_table";

	while (true){
		read_from_file_until(vis,ol,1024,'\n');
		if(strstr(ol,t)) break;
	}
	for(a=0;a<16;a++){
		pa[a]=ol[a];
	}
	pa[16]='\0';
printk(KERN_ALERT "Address: %s \n",pa);
unsigned long sa;
sscanf(pa,"%lx",&sa);
sct=(void*)sa;
printk(KERN_ALERT "fork_sys_call : %px\n", sct[__NR_fork]);
close_file(vis);
kfree(ol);
return 0;}
MODULE_LICENSE("GPL");
void cleanup_module ()
{
	printk (KERN_ALERT "Bye, Done! \n");
}
