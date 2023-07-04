kernel
void
ArrayMult( global const float *dA, global const float *dB, global float *dAnswer)
{
	int gid = get_global_id( 0 );
    int block_size = get_local_size(0);
    int id = get_local_id(0); 
    int work_group_id = get_group_id(0);
    int num_blocks = get_global_size(0)/block_size;

    for(int i=0;i<block_size;i++){
        dAnswer[work_group_id] += dA[id*block_size+i] * dB[id*block_size+i];
    }
    barrier( CLK_LOCAL_MEM_FENCE );
    float total = 0.f;
    for(int i=0; i<num_blocks;i++){
        total += dAnswer[i];
    }
}
