struct Storage(u32);
impl Drop for Storage { fn drop(&mut self) { print!("{} ",40+self.0); } }
struct Value { storage: Storage }
impl Drop for Value { fn drop(&mut self) { print!("{} ",30+self.storage.0); } }
fn acquire(id:u32)->Storage { print!("{} ",10+id); Storage(id) }
fn initialize(storage:Storage,fail:bool)->Result<Value,()> {
 print!("{} ",20+storage.0);
 if fail { Err(()) } else { Ok(Value{storage}) }
}
fn construct(fail:bool)->Result<(),()> {
 let _a=initialize(acquire(0),false)?;
 let _b=initialize(acquire(1),false)?;
 let _d=initialize(acquire(3),false)?;
 let _c=initialize(acquire(2),fail)?;
 Ok(())
}
fn main(){for fail in [false,true] { print!("OCF_RUST {} ",if fail {"partial"} else {"success"});let result=construct(fail);println!("{}",if result.is_ok(){"OK"}else{"ERR"});}}
