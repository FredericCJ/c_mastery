use std::cell::Cell;
struct Owner<'a>(&'a Cell<u32>);
impl Drop for Owner<'_> {fn drop(&mut self) {self.0.set(self.0.get()+1);}}
fn main() {
    let dropped=Cell::new(0);
    let owned:Result<Owner<'_>,(u32,usize)>=Ok(Owner(&dropped));
    let moved=owned.map(|x|x);
    #[cfg(erc_bad_copy)] let _again=owned.map(|x|x);
    assert_eq!(dropped.get(),0);drop(moved);assert_eq!(dropped.get(),1);
    let calls=Cell::new(0);
    let error:Result<u32,(u32,usize)>=Err((3,7));
    let result=error.and_then(|x| {calls.set(calls.get()+1);Ok(x+1)});
    assert_eq!(calls.get(),0);assert_eq!(result,Err((3,7)));
    let result=error.and({calls.set(calls.get()+1);Ok(4)});
    assert_eq!(calls.get(),1);assert_eq!(result,Err((3,7)));
    let cause=result.map_err(|inner|(inner,(9u32,11usize)));
    assert_eq!(cause,Err(((3,7),(9,11))));
    println!("ERC_RUST consume-once drop-one lazy-zero eager-one nested-cause-offset");
}
