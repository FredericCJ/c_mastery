fn main() {
    let mut bytes = vec![19u8, 27];
    let view = &bytes[..];
    assert_eq!(view[0], 19);
    bytes.push(77);
    assert_eq!(bytes[2], 77);
    println!("DT_RUST_PASS");
}
