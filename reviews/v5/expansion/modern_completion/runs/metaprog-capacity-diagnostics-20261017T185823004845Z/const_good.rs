struct Ring<const N:usize>{values:[u32;N]}
fn main(){let r=Ring::<3>{values:[1,2,3]};assert_eq!(r.values.len(),3);}
