macro_rules! record { ($name:ident { $($field:ident : $ty:ty),* }) => { #[derive(PartialEq)] struct $name { $($field:$ty),* } impl $name { fn names()->Vec<&'static str>{vec![$(stringify!($field)),*]} } }; }
record!(Record{code:u16,value:u32,note:i32,quality:u16});
fn main(){let a=Record{code:1,value:2,note:3,quality:4};let b=Record{quality:5,..a};assert!(a!=b);assert_eq!(Record::names(),vec!["code","value","note","quality"]);println!("MPS_RUST declared-macro-registry derived-field-equality");}
