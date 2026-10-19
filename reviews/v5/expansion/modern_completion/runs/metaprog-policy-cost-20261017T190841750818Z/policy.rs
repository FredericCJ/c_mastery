fn fold<const SAT:bool>(input:&[u32],limit:u32)->Result<u32,()>{let mut sum=0;for x in input{if *x>limit-sum{if !SAT{return Err(());}sum=limit;}else{sum+=*x;}}Ok(sum)}
fn main(){assert_eq!(fold::<false>(&[7,5],10),Err(()));assert_eq!(fold::<true>(&[7,5],10),Ok(10));assert_eq!(fold::<true>(&[],0),Ok(0));println!("MPP_RUST const-bool numeric-policy assertions");}
