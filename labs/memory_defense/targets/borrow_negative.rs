fn main() {
    let mut bytes = vec![19u8, 27];
    let view = &bytes[..];
    bytes.push(77);
    println!("{}", view[0]);
}
