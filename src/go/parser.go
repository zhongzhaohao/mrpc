package mrpc

type Parser interface {
    FromString(string) error
    ToString() (string, error)
}