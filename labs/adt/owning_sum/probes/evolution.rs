#![allow(dead_code)]
enum Value { Empty, Number(u32), Pair(u32,u32),
    #[cfg(added)] Text(String) }
fn measure(value: Value)->usize {
    match value {
        Value::Empty=>0,
        Value::Number(x)=>x as usize,
        Value::Pair(x,y)=>(x as usize)+(y as usize),
        #[cfg(repaired)] Value::Text(s)=>s.len(),
        #[cfg(defaulted)] _=>0,
    }
}
fn main(){
    #[cfg(not(added))] assert_eq!(measure(Value::Pair(1,2)),3);
    #[cfg(all(added,defaulted))] assert_eq!(measure(Value::Text(String::from("abc"))),0);
    #[cfg(all(added,repaired))] assert_eq!(measure(Value::Text(String::from("abc"))),3);
}
