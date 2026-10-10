"""Early structural feedback on saved authoring batches, without accepting partial coverage."""
from common import ROOT, load, dump
from validate import check_data

def main():
    data=load(); errors,coverage=check_data(*data)
    local=[e for e in errors if not e.startswith('Module set differs:') and e!='Objective coverage must match every unique canonical objective']
    report={'partial_only':True,'local_contracts_passed':not local,'saved_modules':len(data[1]),'saved_projects':len(data[2]),'errors':local}
    dump(ROOT/'reviews/partial_batch_checks.json',report)
    print(report)
    if local: raise SystemExit(1)

if __name__=='__main__':main()
