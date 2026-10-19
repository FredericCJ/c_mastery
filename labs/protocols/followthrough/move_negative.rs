struct Recovery { bytes: Vec<u8> }
fn consume(_: Recovery) {}
fn main() { let owner = Recovery { bytes: vec![1,2,3] }; consume(owner); println!("{}",owner.bytes.len()); }
