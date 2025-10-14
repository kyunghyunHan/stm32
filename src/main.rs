use tiktoken_rs::get_bpe_from_model;

fn main() {
    // GPT-4 기준 토큰화기 불러오기
    let bpe = get_bpe_from_model("gpt-4").unwrap();
    let tokens = bpe.encode_with_special_tokens("안녕하세요, 토큰화 예제입니다.");

    println!("{:?}", tokens);
    println!("토큰 개수: {}", tokens.len());
}
