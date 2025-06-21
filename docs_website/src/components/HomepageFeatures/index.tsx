import type {ReactNode} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

type FeatureItem = {
  title: string;
  Svg: React.ComponentType<React.ComponentProps<'svg'>>;
  description: ReactNode;
};

const FeatureList: FeatureItem[] = [
  {
    title: 'AI-First Design',
    Svg: require('@site/static/img/asthra_ai_first.svg').default,
    description: (
      <>
        The world's first programming language designed from the ground up for AI code generation.
        Minimal, unambiguous syntax with predictable patterns that AI models can learn and apply consistently.
      </>
    ),
  },
  {
    title: 'Safe Systems Programming',
    Svg: require('@site/static/img/asthra_safe_systems.svg').default,
    description: (
      <>
        Memory safety without garbage collection overhead. Safe C interoperability through explicit 
        ownership annotations. Unsafe blocks for precise control when needed.
      </>
    ),
  },
  {
    title: 'Production Ready',
    Svg: require('@site/static/img/asthra_production_ready.svg').default,
    description: (
      <>
        100% test success rate across 31 categories. Complete language implementation with 
        advanced features like concurrency, pattern matching, and FFI integration.
      </>
    ),
  },
  {
    title: 'Smart Optimization',
    Svg: require('@site/static/img/asthra_smart_optimization.svg').default,
    description: (
      <>
        Revolutionary smart compiler that enables AI-friendly value semantics to automatically 
        compile to C-level performance through intelligent pattern recognition.
      </>
    ),
  },
  {
    title: 'Built-in Concurrency',
    Svg: require('@site/static/img/asthra_concurrency.svg').default,
    description: (
      <>
        Native spawn/await syntax for deterministic task coordination. Channels and coordination 
        primitives built into the language for safe concurrent programming.
      </>
    ),
  },
  {
    title: 'Deterministic by Default',
    Svg: require('@site/static/img/asthra_deterministic.svg').default,
    description: (
      <>
        Reproducible builds and predictable runtime behavior. Non-deterministic operations 
        explicitly marked with annotations for clear intent and debugging.
      </>
    ),
  },
];

function Feature({title, Svg, description}: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <Svg className={styles.featureSvg} role="img" />
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): ReactNode {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          <div className="col col--12">
            <div className="text--center margin-bottom--lg">
              <Heading as="h2">Why Choose Asthra?</Heading>
              <p className="hero__subtitle">
                Combining the best of systems programming with AI-first design principles
              </p>
            </div>
          </div>
        </div>
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
