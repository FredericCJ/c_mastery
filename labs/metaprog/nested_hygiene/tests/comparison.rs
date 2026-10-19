#![recursion_limit="16"]
macro_rules! zipped {($( $a:ident ),* ; $( $b:ident ),*) => { [$(($a,$b)),*] };}
#[cfg(bad_length)]
fn main(){let (a,b,c)=(1,2,3);let _=zipped!(a,b;c);}
#[cfg(bad_scope)]
fn main(){macro_rules! scope {(define)=>{let hidden=1;};(refer)=>{let _=hidden;};}scope!(define);scope!(refer);}
#[cfg(bad_recursion)]
fn main(){macro_rules! recurse {()=>{recurse!()};}recurse!();}
#[cfg(not(any(bad_length,bad_scope,bad_recursion)))]
fn main(){
    let x=1;fn helper()->i32{10}assert_eq!(helper(),10);
    macro_rules! mixed {()=>{(x,helper())};}
    {let x=2;fn helper()->i32{20}assert_eq!(x,2);assert_eq!(mixed!(),(1,20));}
    let (a,b,c,d)=(1,2,3,4);assert_eq!(zipped!(a,b;c,d),[(1,3),(2,4)]);
    let caller=7;macro_rules! inner {($value:ident)=>{{let caller=99;($value,caller)}};}macro_rules! outer {($value:ident)=>{inner!($value)};}
    assert_eq!(outer!(caller),(7,99));
    macro_rules! explicit {($name:ident)=>{let $name=9;};}explicit!(caller);assert_eq!(caller,9);
    println!("MPH_RUST mixed-site-lookup equal-length-repetition nested-argument-context explicit-identifier-binding");
}
