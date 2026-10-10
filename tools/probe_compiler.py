"""Small, bounded compiler/library probes; no general conformance or target claim."""
import subprocess
from common import ROOT,dump,write

PROBES={
 'c17_baseline':('c17','#include <stdint.h>\n_Static_assert(sizeof(uint32_t)==4,"width");\nint main(void){return 0;}\n'),
 'c23_nullptr':('c23','int main(void){int *p=nullptr;return p!=nullptr;}\n'),
 'c23_typeof_auto':('c23','int main(void){auto x=3;typeof(x) y=4;return x+y!=7;}\n'),
 'c23_constexpr':('c23','constexpr int k=7;\nint main(void){return k!=7;}\n'),
 'c23_bitint':('c23','int main(void){unsigned _BitInt(17) x=3;return x!=3;}\n'),
 'c23_checked_arithmetic':('c23','#include <stdckdint.h>\nint main(void){int r;return ckd_add(&r,3,4)||r!=7;}\n'),
 'c23_bit_operations':('c23','#include <stdbit.h>\nint main(void){return stdc_count_ones(3u)!=2;}\n'),
}

def command(arguments):
    return subprocess.run(['wsl','-d','Ubuntu','--exec',*arguments],capture_output=True,encoding='utf-8',errors='replace')

def main():
    folder=ROOT/'build/probes';folder.mkdir(parents=True,exist_ok=True)
    linux='/mnt/f/Learning/Curricula/c_mastery/build/probes'
    version=command(['gcc','--version']).stdout.splitlines()[0]
    libc=command(['getconf','GNU_LIBC_VERSION']).stdout.strip()
    records=[]
    for name,(mode,code) in PROBES.items():
        write(folder/(name+'.c'),code)
        built=command(['gcc','-std='+mode,'-Wall','-Wextra','-Werror','-pedantic-errors',linux+'/'+name+'.c','-o',linux+'/'+name])
        run=command([linux+'/'+name]) if built.returncode==0 else None
        record={'probe':name,'mode':mode,'flags':['-std='+mode,'-Wall','-Wextra','-Werror','-pedantic-errors'],'compiled':built.returncode==0,'run_exit':run.returncode if run else None,'diagnostic':built.stderr[-2000:] if built.returncode else '', 'source':code}
        records.append(record)
    dump(ROOT/'reviews/compiler_probes.json',{'compiler':version,'library':libc,'environment':'WSL Ubuntu Linux x86-64','records':records,'scope':'Only the shown snippets, flags and library installation. No Clang, C conformance, timing or MCU execution claim.'})
    print({'compiler':version,'library':libc,'results':[(r['probe'],r['compiled'],r['run_exit']) for r in records]})

if __name__=='__main__':main()
