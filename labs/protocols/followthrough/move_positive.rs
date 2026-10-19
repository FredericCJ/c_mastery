struct Recovery { bytes: Vec<u8> }
fn consume(owner: Recovery) -> usize { owner.bytes.len() }
fn main() { let owner = Recovery { bytes: vec![1,2,3] }; assert_eq!(consume(owner),3); println!("PF_RUST_PASS"); }
